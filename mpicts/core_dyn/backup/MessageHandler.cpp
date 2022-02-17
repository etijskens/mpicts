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
//    MessageHandlerBase::
//    MessageHandlerBase()
//    {
//        theMessageHandlerRegistry.registerMessageHandler(this);
//    }

 //------------------------------------------------------------------------------------------------
    MessageHandlerBase::
    ~MessageHandlerBase()
    {
        if constexpr(::mpi::_debug_ && _debug_)
            prdbg( "~MessageHandlerBase()" );
    }
 //------------------------------------------------------------------------------------------------

    void
    MessageHandlerBase::
    addMessage(int destination, OptionalMessageData const* md)
    {
        theTransmitter.addMessage(*this, destination, md);
    }

 //------------------------------------------------------------------------------------------------

    size_t // buffer size in number of bytes.
    MessageHandlerBase::
    computeBufferSize()
    {
        return messageItemList_.computeBufferSize();
    }

 //------------------------------------------------------------------------------------------------
    void
    MessageHandlerBase::
    writeBuffer( void* pBuffer ) const
    {
        messageItemList_.write(pBuffer);
    }

 //------------------------------------------------------------------------------------------------
    void
    MessageHandlerBase::
    readBuffer( void* pBuffer ) const
    {
        messageItemList_.read(pBuffer);
    }

//------------------------------------------------------------------------------------------------
}// namespace mpi