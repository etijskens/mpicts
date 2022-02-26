#ifndef PARTICLECONTAINER_H
#define PARTICLECONTAINER_H
#include <Eigen/Geometry> //?

#include "mpicts.h"
using namespace mpi;

namespace mpacts
{//---------------------------------------------------------------------------------------------------------------------
    typedef Eigen::Matrix<float, 3, 1, Eigen::DontAlign> vec_t;
    typedef float real_t;

    class ParticleContainer;
 //---------------------------------------------------------------------------------------------------------------------
    template<typename T>
    class ParticleArray : public std::vector<T>
 //---------------------------------------------------------------------------------------------------------------------
    {
    private: // data members
        std::string name_;
        ParticleContainer& pc_;
    public:
        ParticleArray(std::string const& name, ParticleContainer& pc)
          : name_(name), pc_(pc)
        {}
        std::string const& name() const { return name_; }
        std::string const& particleContainer() const { return pc_; }
    };

 //---------------------------------------------------------------------------------------------------------------------
    class ParticleContainer
 //---------------------------------------------------------------------------------------------------------------------
    {
        static bool const _debug_ = false;
        std::vector<bool> alive_;
    public:
        ParticleArray<real_t> r;
        ParticleArray<real_t> m;
//        std::vector<vec_t>   x;

        ParticleContainer(int size);

        void prdbg();

        inline
        size_t // the size of a particle container.
        size() const {
            return alive_.size();
        }

     // Grow the arrays by a factor 1.5, and return the position of the first new element
     // (which is not alive, obviously).
        int grow();

     // Find an index for a new particle
        mpi::Index_t add();

     // remove an element
        inline void remove(int i) {
            alive_[i] = false;
        }

     // test if alive
        inline bool is_alive(int i) const {
            return alive_[i];
        }
    };
}// namespace mpacts


namespace mpi
{//------------------------------------------------------------------------------------------------
    using namespace mpacts;

 //------------------------------------------------------------------------------------------------
    class PcMessageData : public MessageData
 //------------------------------------------------------------------------------------------------
    {
        Indices_t indices_;
        bool move_;
    public:
        PcMessageData
          ( int src   // MPI source rank
          , int dst   // MPI destination rank
          , Key_t key // MessageHandler key
          , Indices_t const& selected // list of selected particles
          , bool move                 // move or copy the selected, has only effect at the sending site
          )
          : MessageData(src, dst, key)
          , indices_(selected)
          , move_(move)
        {}

        PcMessageData  // Create MessageData for receiving a message
          ( int src  // MPI source rank from whom to receive
          , size_t i // location in MessageHeaderContainer for MPI rank src
          )
          : MessageData(src, i)
        {}

        bool move() const { return move_; }
        bool move()       { return move_; }
        Indices_t const& indices() const { return indices_; }
        Indices_t      & indices()       { return indices_; }
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
                prdbg(tostr("~MessageItem<ParticleContainer>()"));
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
                Lines_t lines = tolines("indices ", pPcMessageData->indices());
                lines.push_back( tostr("move = ", pPcMessageData->move(), (pPcMessageData->move() ? "(particles are deleted)" : "(particles are copied)")) );
                prdbg( tostr("MessageItem<ParticleContainer>::write(ptr)"), lines );
            }

            size_t nParticles = pPcMessageData->indices().size();
            ::mpi::write( nParticles, pos );
            if( pPcMessageData->move() )
            {// Remove the particles from the ParticleContainer
                for( auto index : pPcMessageData->indices() )
                    ptr_pc_->remove(index);
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

            Index_t n;
            ::mpi::read( n, pos );
         // create n new particles
            Indices_t & indices = pPcMessageData->indices();
            indices.resize(n);
            for( size_t i = 0; i < n; ++i )
                indices[i] = ptr_pc_->add();

            if constexpr(::mpi::_debug_ && _debug_) {
                prdbg( tostr("MessageItem<ParticleContainer>::read(ptr)"), tolines("new indices", indices) );
            }
        }

     // The number of bytes that this MessageItem will occupy in a MessageBuffer. As this MessageItem only conveys
     // the number of particles to be transferred, it is just sizeof()
        virtual size_t computeItemBufferSize
          ( MessageData const* /*pMessageData*/
          ) const
        {
            return sizeof(size_t); // this MessageItem only transfers the number of particles to be transferred
        }

        virtual INFO_DECL
        {
            std::stringstream ss;
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
                prdbg(tostr("~MessageItem<ParticleArray<T=", typeid(T).name(), ">>()"));
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
                prdbg( tostr("MessageItem<ParticleArray<T=", typeid(T).name(), ">>::write(ptr)")
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
                prdbg( tostr("MessageItem<ParticleArray<T=", typeid(T).name(), ">>::read(ptr)")
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
        virtual INFO_DECL
        {
            std::stringstream ss;
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
    public:
     // ctor
        PcMessageHandler(ParticleContainer& pc);

        void
        addSendMessage
          ( int destination
          , Indices_t const & indices
          , bool move
          );

    };

 //-------------------------------------------------------------------------------------------------
}// namespace mpi
#endif // PARTICLECONTAINER_H