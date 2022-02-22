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
{
    using namespace mpacts

 //------------------------------------------------------------------------------------------------
    struct PcExtraMessageData : public ExtraMessageData
 //------------------------------------------------------------------------------------------------
    {
        Indices_t indices;
        bool move;
        PcExtraMessageData(Indices_t const& i, bool m)
          : indices(i)
          , move(m)
        {}
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
        std::vector<PcMessageItemData> data_;
    public:
     // ctor
        MessageItem
          ( ParticleContainer& pc
          )
          : ptr_pc_(&pc)
        {}

     // Set the selection of particles and whether to move or copy them,
        void
        addMessage
          ( int destination
          , Indices_t& indices // list of particles to be moved/copied.
          , bool move          // choose between moving or copying the particles to the other proces
          )
        {
            data_.push_back(PcMessageItemData(destination, indices, move));
        }

     // indices accessor
        Indices_t const& indices() const { return indices_; }
        Indices_t      & indices()       { return indices_; }

     // dtor
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
          ( void*& dst // pointer in the MessageBuffer where this item must be written.
                       // Is advanced by the number of bytes written on return
          ) const
       {
            if constexpr(::mpi::_debug_ && _debug_) {
                Lines_t lines = tolines("indices ", indices());
                lines.push_back( tostr("move = ", move_, (move_ ? "(particles are deleted)" : "(particles are copied)")) );
                prdbg( tostr("MessageItem<ParticleContainer>::write(dst)"), lines );
            }

            size_t nParticles = indices_.size();
            ::mpi::write( nParticles, dst );
            if( move_ )
            {// Remove the particles from the ParticleContainer
                for( auto index : indices_ )
                    ptr_pc_->remove(index);
            }
        }

     // Read the number of selected particles and create as many new particles in the ParticleContainer.
        virtual void read(void*& src)
        {
            Index_t n;
            ::mpi::read( n, src );
         // create n new particles
            indices_.resize(n);
            for( size_t i = 0; i < n; ++i )
                indices_[i] = ptr_pc_->add();

            if constexpr(::mpi::_debug_ && _debug_) {
                prdbg( tostr("MessageItem<ParticleContainer>::read(src)"), tolines("new indices", indices_) );
            }
        }

     // The number of byte that this MessageItem will occupy in a MessageBuffer
        virtual size_t computeBufferSize() const {
            return sizeof(size_t);
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
          ( void*& dst // pointer in the MessageBuffer where this item must be written.
                       // Is advanced by the number of bytes written on return
          ) const
       {
            if constexpr(::mpi::_debug_ && _debug_) {
                prdbg( tostr("MessageItem<ParticleArray<T=", typeid(T).name(), ">>::write(dst)")
                     , tolines(ptr_pa_->name(), *ptr_pa_, ptr_pc_message_item_->indices() )
                     );
            }
            for( auto index : ptr_pc_message_item_->indices() )
                ::mpi::write( (*ptr_pa_)[index], dst );
        }

     // Read the selected particles from src
        virtual
        void
        read
          ( void*& src // pointer in the MessageBuffer where this item must be written.
                       // Is advanced by the number of bytes written on return
          )
        {
            for( auto index : ptr_pc_message_item_->indices() )
                ::mpi::read( (*ptr_pa_)[index], src );

            if constexpr(::mpi::_debug_ && _debug_) {
                prdbg( tostr("MessageItem<ParticleArray<T=", typeid(T).name(), ">>::read(src)")
                     , tolines(ptr_pa_->name(), *ptr_pa_, ptr_pc_message_item_->indices() )
                     );
            }
        }

     // The number of bytes that this MessageItem will occupy in a MessageBuffer
        virtual size_t computeBufferSize() const {
            return ptr_pc_message_item_->indices().size() * sizeof(T);
        }
    };
}// namespace mpi
#endif // PARTICLECONTAINER_H