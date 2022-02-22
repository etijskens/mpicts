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
    registerMessageHandler(MessageHandler* pMessageHandler)
    {
        registry_[counter_] = pMessageHandler;
        pMessageHandler->key_ = counter_;
        ++counter_;
    }

    INFO_DEF(MessageHandlerRegistry)
    {
        std::stringstream ss;
        ss<<indent<<"MessageHandlerRegistry.info("<<title<<") : ";
        if( registry_.size() ) {
            for( auto const& entry : registry_ ) {
                ss<<entry.second->info(indent+"  ");
            }
        } else {
            ss<<"( empty )";
        }
        return ss.str();
    }

 //------------------------------------------------------------------------------------------------
 // MessageHandler implementation
 //------------------------------------------------------------------------------------------------
    MessageHandlerRegistry MessageHandler::theMessageHandlerRegistry;

    MessageBufferPool      MessageHandler::theMessageBufferPool;

    MessageHandler::
    MessageHandler()
    {
        theMessageHandlerRegistry.registerMessageHandler(this);
    }

    MessageHandler::
    ~MessageHandler()
    {
        if constexpr(::mpi::_debug_ && _debug_) prdbg( "~MessageHandler()" );

     // destroy the MessageData objects in messageDataList_:
        for( auto pMessageData : messageDataList_ )
        {
            if( pMessageData ) {
                if constexpr(::mpi::_debug_ && _debug_) prdbg(tostr("delete pMessageData=", pMessageData));
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
        assert( destination < mpi::size
             && "Invalid MPI rank for destination."
              ); // https://stackoverflow.com/questions/3692954/add-custom-messages-in-assert/26984456
         // We allow sending to self rank for testing.

        messageDataList_.push_back(new MessageData( mpi::rank, destination, this->key_ ));
    }

 //------------------------------------------------------------------------------------------------

    size_t // buffer size in number of bytes.
    MessageHandler::
    computeMessageBufferSize()
    {
        return messageItemList_.computeMessageBufferSize();
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

    INFO_DEF(MessageHandler)
    {
        std::stringstream ss;
        ss<<indent<<"MessageHandler.info("<<title<<") : { key='"<<key_<<"':"
          <<messageItemList().info(indent + "  ")
          <<indent<<'}';
        return ss.str();
    }

    STATIC_INFO_DEF(MessageHandler)
    {
        std::stringstream ss;
        ss<<"\nMessageHandler::static_info("<<title<<"):"
          <<theMessageHandlerRegistry.info(indent + "  ")
          <<theMessageBufferPool     .info(indent + "  ")
          ;

        return ss.str();
    }

//------------------------------------------------------------------------------------------------
}// namespace mpi