#ifndef MESSAGE_H
#define MESSAGE_H

#include "memcpy_able.h"
#include "MessageData.h"
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
    // write the message item to pos
        virtual void write( void*& pos, MessageData* pMessageData ) const = 0;
    // read the message item from pos
        virtual void read ( void*& pos, MessageData* pMessageData ) = 0;
    // get the size of the message item (in bytes)
        virtual size_t computeItemBufferSize( MessageData const* pMessageData ) const = 0;

        virtual INFO_DECL = 0;
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

        virtual
        ~MessageItem()
        {
            if constexpr(::mpi::_debug_ && _debug_) {
                prdbg( tostr("\n~MessageItem<T=", typeid(T).name(), ">() : this=", this) );
            }
        }

     // Write the content of ptrT_ to pos and advance pos
        virtual void write( void*& pos, MessageData* pMessageData ) const
       {
            if constexpr(::mpi::_debug_ && _debug_) {
                prdbg( tostr(info(), "::write()") );
            }
            ::mpi::write( *ptrT_, pos );
        }

     // Read the content of ptrT_ from src
        virtual void read( void*& pos, MessageData* pMessageData )
        {
            ::mpi::read( *ptrT_, pos );

            if constexpr(::mpi::_debug_ && _debug_) {
                prdbg( tostr(info(), "::read()") );
            }
        }

     // Compute the size (in bytes) that *ptrT_ will occupy in a message.
        virtual size_t computeItemBufferSize
          ( MessageData const* /*pMessageData*/
          ) const
        {
            return ::mpi::computeItemBufferSize(*ptrT_);
        }

        virtual INFO_DECL
        {
            std::stringstream ss;
            ss<<indent<<"MessageItem("<<title<<")<T="<<typeid(T).name()<<">";
            return ss.str();
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

        size_t size() const { return list_.size(); }

     // Add a simple item to the message
        template<typename T>
        MessageItem<T>* // Return the constructed MessageItem.
                        // This is occasionally needed when other MessageItems need to know about it.
                        // E.g. the ParticleArray MessageItem needs a ParticleContainer MessageItem.
        push_back
          ( T& t // object to incorporate in the message
          )
        {
            MessageItem<T>* p = new MessageItem<T>(t);
            list_.push_back(p);
            return p;
        }

     // Add an item to the message that needs access to another MessageItem.
        template<typename T>
        MessageItem<T>* // Return the constructed MessageItem.
                        // This is occasionally needed when other MessageItems need to know about it.
                        // E.g. the ParticleArray MessageItem needs a ParticleContainer MessageItem.
        push_back
          ( T& t // object to incorporate in the message
          , MessageItemBase* pOtherItem // a MessageItem that contains information needed by the MessageItem to be constructed
          )
        {
            MessageItem<T>* p = new MessageItem<T>(t, pOtherItem);
            list_.push_back(p);
            return p;
        }

     // Write the message to a buffer at ptr.
        void write(MessageData* pMessageData) const;

     // Read the message from ptr in buffer
        void read(MessageData* pMessageData);

     // Compute the number of bytes the message occupies in a MessageBuffer.
        size_t
        computeMessageBufferSize
          ( MessageData* pMessageData
          ) const;

        INFO_DECL;
    };
 //-------------------------------------------------------------------------------------------------
}// namespace mpi

#endif // MESSAGE_H
