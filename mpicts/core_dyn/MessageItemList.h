#ifndef MESSAGE_H
#define MESSAGE_H

#include "memcpy_able.h"
#include <string>
#include <iostream>
#include <sstream>

namespace mpi
{//-------------------------------------------------------------------------------------------------
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
        virtual size_t computeByteSize() const = 0;
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
        virtual size_t computeByteSize() const {
            return ::mpi::computeByteSize(*ptrT_);
        }
    };
  //-------------------------------------------------------------------------------------------------
    class MessageItemList // FIFO list of MessageItem<T>* objects.
 // provides member functions to write, read and obtain the size of a message.
 //-------------------------------------------------------------------------------------------------
    {
        static bool const _debug_ = true;

        std::vector<MessageItemBase*> list_;

    public:
        ~MessageItemList();

     // Add an item to the message (behaves as FIFO)
        template<typename T> 
        MessageItem<T>* // Return the constructed MessageItem. Occasionally needed.
        push_back
          ( T& t // object to incorporate in the message
          )
        {
            MessageItem<T>* p = new MessageItem<T>(t);
            list_.push_back(p);
            return p;
        }

     // Add an item to the message
        template<typename T>
        MessageItem<T>* // Return the constructed MessageItem. Occasionally needed.
        push_back
          ( T& t // object to incorporate in the message
          , MessageItemBase* ptr_mi // a MessageItem that contains information needed by this MessageItem
                                    // e.g. the ParticleArray MessageItem needs a ParticleContainer MessageItem.
          )
        {
            MessageItem<T>* p = new MessageItem<T>(t, ptr_mi);
            list_.push_back(p);
            return p;
        }

     // Write the message to ptr in the MessageBuffer
        void write(void*& ptr) const;

     // Read the message from ptr in the MessageBuffer
        void read (void*& ptr);

     // Compute the number of bytes the message occupies in a MessageBuffer.
        size_t computeBufferSize() const;

    };
 //-------------------------------------------------------------------------------------------------
}// namespace mpi

#endif // MESSAGE_H
