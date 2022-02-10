#ifndef MESSAGE_H
#define MESSAGE_H

#include "memcpy_able.h"
#include <string>
#include <iostream>
#include <sstream>

namespace mpi
{// Although nothing in this file uses MPI, it is necessary machinery for the MPI messageing 
 // system that we need
 
 //-------------------------------------------------------------------------------------------------
 // Convert a size in bytes, ensuring that the result is rounded up to Boundary. The result is
 // expressed in word of Unit bytes. E.g. 
 //     convertSizeInBytes<8>(4)   -> 8 : the smallest 8 byte boundary >= 4 bytes is 8 bytes
 //     convertSizeInBytes<8,8>(4) -> 8 : the smallest 8 byte boundary >= 4 bytes is 1 8-byte word
 //     convertSizeInBytes<8,2>(4) -> 8 : the smallest 8 byte boundary >= 4 bytes is 4 2-byte words
    template<size_t Boundary, size_t Unit=1>
    Index_t convertSizeInBytes(Index_t bytes) {
        return ((bytes + Boundary - 1) / Boundary) * (Boundary / Unit);
    }

 
 //-------------------------------------------------------------------------------------------------
    class MessageItemBase
 //-------------------------------------------------------------------------------------------------
        {
    public:
        virtual ~MessageItemBase() {}
    // write the message item to dst
        virtual void write(void*& dst) const = 0;
    // read the message item from src
        virtual void read (void*& src)       = 0;
    // get the size of the message item (in bytes)
        virtual size_t messageSize() const = 0;
    };

 //-------------------------------------------------------------------------------------------------
    template <typename T>
    class MessageItem : public MessageItemBase
 //-------------------------------------------------------------------------------------------------
    {
        static const bool _debug_ = true; // write debug output or not

     // data members
        T* ptrT_;

    public:
        MessageItem(T& t) // Contiguous
          : ptrT_(&t)
          {}

        ~MessageItem()
        {
            if constexpr(::mpi::_debug_ && _debug_) {
                prdbg(tostr("~MessageItem<T=", typeid(T).name(), ">() : this=", this));
            }
        }

     // Write the content of ptrT_ to dst
        virtual void write( void*& dst ) const
       {
            if constexpr(::mpi::_debug_ && _debug_) {
                prdbg( tostr("MessageItem<T=", typeid(T).name(), ">::write()") );
            }
            ::mpi::write( *ptrT_, dst );
        }

     // Read the content of ptrT_ from src
        virtual void read(void*& src)
        {
            ::mpi::read( *ptrT_, src );

            if constexpr(::mpi::_debug_ && _debug_) {
                prdbg( tostr("MessageItem<T=", typeid(T).name(), ">::read()") );
            }
        }

     // Size that *ptrT_ will occupy in a message, in bytes
        virtual size_t messageSize() const {
            return ::mpi::messageSize(*ptrT_);
        }
    };
 //-------------------------------------------------------------------------------------------------
 // Specialisation
    template <>
    class MessageItem<ParticleContainer> : public MessageItemBase
 //-------------------------------------------------------------------------------------------------
    {
        static const bool _debug_ = true; // write debug output or not

    private: // data members
        ParticleContainer* ptr_pc_;
        Indices_t indices_;
        bool move_;
    public:
     // ctor
        MessageItem
          ( ParticleContainer& pc
          )
          : ptr_pc_(&pc)
        {}

     // Set the selection of particles ane whether to move or copy them,
        void
        select
          ( Indices_t& indices // list of particles to be moved/copied.
          , bool move          // choose between moving or copying the particles to the other proces
          )
        {
            indices_ = indices; // make a copy
            move_ = move;
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
        virtual size_t messageSize() const {
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
        virtual size_t messageSize() const {
            return ptr_pc_message_item_->indices().size() * sizeof(T);
        }
    };
 //-------------------------------------------------------------------------------------------------
    class Message // FIFO list of MessageItem<T>* objects.
 // provides member functions to write, read and obtain the size of a message.
 //-------------------------------------------------------------------------------------------------
    {
        static bool const _debug_ = true;

        std::vector<MessageItemBase*> coll_;

    public:
        ~Message();

     // Add an item to the message (behaves as FIFO)
        template<typename T> 
        MessageItem<T>* // Return the constructed MessageItem. Occasionally needed.
        push_back
          ( T& t // object to incorporate in the message
          )
        {
            MessageItem<T>* p = new MessageItem<T>(t);
            coll_.push_back(p);
            return p;
        }

     // Add an item to the message (behaves as FIFO)
        template<typename T>
        MessageItem<T>* // Return the constructed MessageItem. Occasionally needed.
        push_back
          ( T& t // object to incorporate in the message
          , MessageItemBase* ptr_mi // a MessageItem that contains information needed by this MessageItem
                                    // e.g. the ParticleArray MessageItem needs a ParticleContainer MessageItem.
          )
        {
            MessageItem<T>* p = new MessageItem<T>(t, ptr_mi);
            coll_.push_back(p);
            return p;
        }

     // Write the message to ptr in the MessageBuffer
        void write(void*& ptr) const;

     // Read the message from ptr in the MessageBuffer
        void read (void*& ptr);

     // The number of bytes the message occupies in the MessageBuffer.
        size_t messageSize() const;

    };
 //-------------------------------------------------------------------------------------------------
}// namespace mpi

#endif // MESSAGE_H
