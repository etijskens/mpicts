#ifndef MPI_PARTICLECONTAINER_H
#define MPI_PARTICLECONTAINER_H
#include <Eigen/Geometry> //?

#include "mpicts.h"

// Here, the true mpacts ParticleContainer and ParticleArray must be included
// this is just a stub
#include "mpacts/ParticleContainer.h"

namespace mpi
{//------------------------------------------------------------------------------------------------
    using namespace mpacts;

 //-------------------------------------------------------------------------------------------------
    enum Mode
 // Operation mode for ParticleContainer Messages
 //-------------------------------------------------------------------------------------------------
    { move // Move particles: the selected particles are destroyed in the sender after sending.
           // The receivers adds the particles.
    , copy // Copy particles: the selected particles are not destroyed in the sender after sending.
           // The receiver adds the particles.
    , add // move or copy on the receiving end.
  // todo: extend move/copy to move/copy/set
    , set  // Overwrite particles at the receiving end. The sender sends a list of particle ids.
           // the MenuItemList is different ?? maybe we need a different PcMessageHandler
           // alternatively, we can send the mode as part of the MenuItem<ParticleContainer>
           // as well as the list of indices if mode==set
    , none
    };

 // The receiver translates the IDs to the corresponding particle indices and overwrites
 // the received arrays with the received array values for the selection.

    std::string
    str( Mode mode )
    {
        switch(mode) {
            case move: return "Move : selected particles are moved from sender to receiver.";
            case copy: return "Copy : selected particles are copied from sender to receiver.";
            case add : return "add : move or copy on the receiving end.";
            case set : return "set : selected particles are overwritten at the receiving end.";
            case none: return "none";
            default:
                assert(false && "Unknwown mode");
        }
    }

 //------------------------------------------------------------------------------------------------
    class PcMessageData : public MessageData
 //------------------------------------------------------------------------------------------------
    {
        Indices_t indices_;
        Mode      mode_;

    public:
        PcMessageData
          ( int src   // MPI source rank
          , int dst   // MPI destination rank
          , Key_t key // MessageHandler key
          , Indices_t const& selected // list of selected particles
          , Mode mode                 // operation mode
          )
          : MessageData(src, dst, key)
          , indices_(selected)
          , mode_(mode)
        {}

        PcMessageData  // Create MessageData for receiving a message
          ( int src  // MPI source rank from whom to receive
          , size_t i // location in MessageHeaderContainer for MPI rank src
          )
          : MessageData(src, i)
          , mode_(none)
        {}

        Mode  mode() const { return mode_; }
        Mode& mode()       { return mode_; }
        Indices_t const& indices() const { return indices_; }
        Indices_t      & indices()       { return indices_; }

        virtual INFO_DECL;
    };

 //------------------------------------------------------------------------------------------------
    template <>
    class MessageItem<ParticleContainer> : public MessageItemBase
 // Specialisation For ParticleContainers
 //-------------------------------------------------------------------------------------------------
    {
        static const bool _debug_ = true; // write debug output or not

    private: // data members
        ParticleContainer* ptr_pc_;
    public:
     // ctor
        MessageItem
          ( ParticleContainer& pc
          )
          : ptr_pc_(&pc)
        {}

     // dtor
        virtual
        ~MessageItem()
        {
            if constexpr(::mpi::_debug_ && _debug_) {
                prdbg(concatenate("~MessageItem<ParticleContainer>()"));
            }
        }

     // Write the number of selected particles to the MessageBuffer. This is all the reader has to know
     // (how many new particles must be created). Remove the particle from the ParticleContainer if requested.
        virtual
        void
        write
          ( void*& pos
          , MessageData* pMessageData
          ) const
       {
            PcMessageData* pPcMessageData = dynamic_cast<PcMessageData*>(pMessageData);
            if constexpr(::mpi::_debug_ && _debug_) {
                prdbg( concatenate( "MessageItem<ParticleContainer>::write(): entering"
                            , pPcMessageData->info()
                ));
            }

            size_t nParticles = pPcMessageData->indices().size();
            ::mpi::write( nParticles, pos );
            ::mpi::write( pPcMessageData->mode(), pos );
            if constexpr(::mpi::_debug_ && _debug_) {
                prdbg( concatenate( "MessageItem<ParticleContainer>::write(): indices.size(), mode written" ));
            }

            if( pPcMessageData->mode() == set )
            {// also write the indices
             // TODO: We ought to write particle IDs, which on the receiving end must be translated back into indices.
                ::mpi::write( pPcMessageData->indices(), pos );

                if constexpr(::mpi::_debug_ && _debug_) {
                    prdbg( concatenate( "MessageItem<ParticleContainer>::write(): indices written" ));
                }
            }

            if( pPcMessageData->mode() == move )
            {// Remove the particles from the ParticleContainer
                for( auto index : pPcMessageData->indices() )
                    ptr_pc_->remove(index);
                if constexpr(::mpi::_debug_ && _debug_) {
                    prdbg( concatenate( "MessageItem<ParticleContainer>::write(): selection removed in sender" ));
                }
            }
            if constexpr(::mpi::_debug_ && _debug_) {
                prdbg( concatenate( "MessageItem<ParticleContainer>::write(): leaving" ));
            }
        }

        virtual
        void
        read // Read the number of selected particles and create as many new particles in the ParticleContainer.
          ( void*& pos
          , MessageData* pMessageData
          )
        {
            PcMessageData* pPcMessageData = dynamic_cast<PcMessageData*>(pMessageData);

            if constexpr(::mpi::_debug_ && _debug_) {
                prdbg( concatenate( "MessageItem<ParticleContainer>::read(): entering"
                            , pPcMessageData->info()
                ));
            }

            Index_t n;
            ::mpi::read( n, pos );
            ::mpi::read( pPcMessageData->mode(), pos );
            if constexpr(::mpi::_debug_ && _debug_) {
                prdbg( concatenate( "MessageItem<ParticleContainer>::read(): n, mode"
                            , pPcMessageData->info()
                ));
            }

            if( pPcMessageData->mode() == set )
            {// read the IDs of the particles to be overwritten
                ::mpi::read( pPcMessageData->indices(), pos );
             // todo: add ID to index translation
                if constexpr(::mpi::_debug_ && _debug_) {
                    prdbg( concatenate( "MessageItem<ParticleContainer>::read(): selection"
                                , pPcMessageData->info()
                    ));
                }
            } else
            {// mode == move|copy
             // create n new particles
                Indices_t & indices = pPcMessageData->indices();
                indices.resize(n);
                for( size_t i = 0; i < n; ++i )
                    indices[i] = ptr_pc_->add();

                if constexpr(::mpi::_debug_ && _debug_) {
                    prdbg( concatenate( "MessageItem<ParticleContainer>::read(): particles added"
                                , pPcMessageData->info()
                    ));
                }
            }

        }

     // The number of bytes that this MessageItem will occupy in a MessageBuffer. As this MessageItem only conveys
     // the number of particles to be transferred, it is just sizeof()
        virtual size_t computeItemBufferSize
          ( MessageData const* pMessageData
          ) const
        {
            PcMessageData const* pPcMessageData = dynamic_cast<PcMessageData const*>(pMessageData);

            size_t nBytes = sizeof(size_t) // the size
                          + sizeof(Mode);  // the mode
            if( pPcMessageData->mode() == set ) {
                nBytes += pPcMessageData->indices().size() * sizeof(Index_t);
            }
            return nBytes;
        }

        ParticleContainer const& particleContainer() const { return *ptr_pc_; }

        virtual INFO_DECL
        {
            std::stringstream ss;
            ss<<indent<<"MessageItem<ParticleContainer>::info("<<title<<") : pc="<<ptr_pc_->name()<<", size="<<ptr_pc_->size();
            return ss.str();
        }
   };
 //-------------------------------------------------------------------------------------------------
 // Specialisation
    template <typename T>
    class MessageItem<ParticleArray<T>> : public MessageItemBase
 //-------------------------------------------------------------------------------------------------
    {
        static const bool _debug_ = true; // write debug output or not

    private: // data members
        ParticleArray<T>* ptr_pa_;
        MessageItem<ParticleContainer>* ptr_pc_message_item_;

    public:
     // ctor
        MessageItem
          ( ParticleArray<T>& pa
          , MessageItemBase* ptr_pc_message_item
          )
          : ptr_pa_(&pa)
          , ptr_pc_message_item_( dynamic_cast<MessageItem<ParticleContainer>*>(ptr_pc_message_item) )
        {}

     // dtor
        virtual
        ~MessageItem()
        {
            if constexpr(::mpi::_debug_ && _debug_) {
                prdbg(concatenate("~MessageItem<ParticleArray<T=", typeid(T).name(), ">>()"));
            }
        }

     // Write the selected array elements to the MessageBuffer
        virtual
        void
        write
          ( void*& pos
          , MessageData* pMessageData
          ) const
       {
            PcMessageData* pPcMessageData = dynamic_cast<PcMessageData*>(pMessageData);

            if constexpr(::mpi::_debug_ && _debug_) {
                prdbg( concatenate("MessageItem<ParticleArray<T=", typeid(T).name(), ">>::write(ptr)")
                     , tolines(ptr_pa_->name(), *ptr_pa_, pPcMessageData->indices() )
                     );
            }
            for( auto index : pPcMessageData->indices() )
                ::mpi::write( (*ptr_pa_)[index], pos );
        }

     // Read the selected particles from ptr
        virtual
        void
        read
          ( void*& pos
          , MessageData* pMessageData
          )
        {
            PcMessageData* pPcMessageData = dynamic_cast<PcMessageData*>(pMessageData);

            for( auto index : pPcMessageData->indices() )
                ::mpi::read( (*ptr_pa_)[index], pos );

            if constexpr(::mpi::_debug_ && _debug_) {
                prdbg( concatenate("MessageItem<ParticleArray<T=", typeid(T).name(), ">>::read(ptr)")
                     , tolines(ptr_pa_->name(), *ptr_pa_, pPcMessageData->indices() )
                     );
            }
        }

     // The number of bytes that this MessageItem will occupy in a MessageBuffer
        virtual size_t computeItemBufferSize
          ( MessageData const* pMessageData
          ) const
        {
            PcMessageData const* pPcMessageData = dynamic_cast<PcMessageData const*>(pMessageData);

            return pPcMessageData->indices().size() * sizeof(T);
        }

        virtual
        INFO_DECL
        {
            std::stringstream ss;
            ss<<indent<<"MessageItem<ParticleArray<"<<typeid(T).name()<<">>::info("<<title<<") : name="<<ptr_pa_->name()
                      <<", pc="<<ptr_pc_message_item_->particleContainer().name();
            return ss.str();
        }
    };

 //-------------------------------------------------------------------------------------------------
    class PcMessageHandler : public MessageHandler
 //-------------------------------------------------------------------------------------------------
    {
    private: // data members
        ParticleContainer& pc_;
        MessageItem<ParticleContainer>* ptr_pc_message_item_;
        Index_t nParticles_;
    protected:
     // ctor
     // TODO: this ctor automatically adds the ParticleArrays to the MessageItemList.
     // TODO: Add a way to selectively add/delete ParticleArrays and a suitable default.
        PcMessageHandler(ParticleContainer& pc);

    public:
     // Create and register a PcMessageHandler (through the proctected ctor)
        static PcMessageHandler& create(ParticleContainer& pc);

        virtual
        void
        addSendMessage
          ( int destination             // destination MPI rank
          , Indices_t const & selection // List of selected particles
          , Mode mode                   // Operation mode
          );

        virtual
        void addRecvMessage(int src, size_t i);
    };

 //-------------------------------------------------------------------------------------------------
}// namespace mpi
#endif // MPI_PARTICLECONTAINER_H