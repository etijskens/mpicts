#include "MessageHandler.h"

namespace mpi
{
 //------------------------------------------------------------------------------------------------
 // MessageHandlerRegistry implementation
 //------------------------------------------------------------------------------------------------

    MessageHandlerRegistry::
    MessageHandlerRegistry() : counter_(0) {}

    void
    MessageHandlerRegistry::
    registerMessageHandler(MessageHandlerBase* messageHandler)
    {
        registry_[counter_] = messageHandler;
        messageHandler->key_ = counter_;
        ++counter_;
    }

    MessageHandlerRegistry theMessageHandlerRegistry;

 //------------------------------------------------------------------------------------------------
 // MessageHandlerRegistry implementation
 //------------------------------------------------------------------------------------------------
    MessageHandlerBase::
    MessageHandlerBase()
    {
        theMessageHandlerRegistry.registerMessageHandler(this);
    }

 //------------------------------------------------------------------------------------------------
    MessageHandlerBase::
    ~MessageHandlerBase()
    {
        if constexpr(::mpi::_debug_ && _debug_)
            prdbg( "~MessageHandlerBase()" );
    }

 //------------------------------------------------------------------------------------------------
    Index_t
    MessageHandlerBase::
    writeMessage(int to_rank)
    {// construct the message, and put the message in the messageBuffer
     // compute the length of the message:
        Index_t sz = convertSizeInBytes<sizeof(Index_t)>(message_.messageSize());
     // allocate memory space for the message in the message buffer, and write the header
     // for the message in the message buffer
        int const from_rank = rank;
        Index_t msg_id = -1;
        void* ptr = theMessageBuffer.allocateMessage( sz, from_rank, to_rank, key_, &msg_id );
     // Write the message in the message buffer
        message_.write(ptr);

        if constexpr(::mpi::_debug_ && _debug_) {
            prdbg
              ( tostr("MessageHandlerBase::postMessage() : headers (current msg_id=", msg_id, ")")
              , theMessageBuffer.headersToStr()
              );
            prdbg
              ( tostr("MessageHandlerBase::postMessage() : message (current msg_id=", msg_id, ")")
              , theMessageBuffer.messageToStr(msg_id)
              );
        }
        return msg_id;
    }

 //------------------------------------------------------------------------------------------------
    bool               // true if the MessageHandlerKey value in the message header
                       // with id msg_id corresponds to this->key_, false otherwise.
                       // in which case the message was not written by this
                       // MessageHandler, and therefore also cannot be read by it.
    MessageHandlerBase::
    readMessage
      ( Index_t msg_id // the message id identifies the message header
      )
    {// Verify that this is the correct MessageHandler for this message
        if( theMessageBuffer.messageHandlerKey(msg_id) != key_)
            return false;

     // Read
        void* ptr = theMessageBuffer.messagePtr(msg_id);
        message_.read(ptr);

        return true;
    }
 //------------------------------------------------------------------------------------------------
}// namespace mpi