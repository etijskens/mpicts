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
    registerMessageHandler(MessageHandler* messageHandler)
    {
        registry_[counter_] = messageHandler;
        messageHandler->key_ = counter_;
        ++counter_;
    }

 // MessageHandlerRegistry theMessageHandlerRegistry;

 //------------------------------------------------------------------------------------------------
 // MessageHandlerRegistry implementation
 //------------------------------------------------------------------------------------------------
    MessageHandler::
    ~MessageHandler()
    {
        if constexpr(::mpi::_debug_ && _debug_)
            prdbg( "~MessageHandler()" );
     // destroy the MessageData objects in messageDataList_:
        for( auto pMessageData : messageDataList_ )
        {
            if( pMessageData ) {
                if constexpr(::mpi::_debug_ && _debug_) prdbg(tostr("delete ", pMessageData));
                delete pMessageData;
            } else {
                if constexpr(::mpi::_debug_ && _debug_) prdbg("pMessageData == nullptr");

            }
        }
    }
 //------------------------------------------------------------------------------------------------

    void
    MessageHandler::
    addMessage(int destination)
    {
        messageDataList_.push_back(new MessageData( mpi::rank, destination, this->key_ ));
    }

 //------------------------------------------------------------------------------------------------

    size_t // buffer size in number of bytes.
    MessageHandler::
    computeBufferSize()
    {
        return messageItemList_.computeBufferSize();
    }

 //------------------------------------------------------------------------------------------------
    void
    MessageHandler::
    writeBuffer( void* pBuffer ) const
    {
        messageItemList_.write(pBuffer);
    }

 //------------------------------------------------------------------------------------------------
    void
    MessageHandler::
    readBuffer( void* pBuffer ) const
    {
        messageItemList_.read(pBuffer);
    }

//------------------------------------------------------------------------------------------------
}// namespace mpi