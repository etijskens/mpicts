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
    void
    MessageHandler::
    postMessages() // Allocate buffers and write the messages to their buffers.
    {
    }

 //------------------------------------------------------------------------------------------------
    void
    MessageHandler::
    transferMessages() // Send and receive the messages
    {
    }

 //------------------------------------------------------------------------------------------------
    void
    MessageHandler::
    readMessages() // Read the messages from the receive buffers
    {
    }

 //------------------------------------------------------------------------------------------------
//    size_t                   // number of bytes the message needs
//    MessageHandler::
//    computeMessageBufferSize // Compute the size (bytes) that a message will occupy when written to a buffer
//      ( size_t i             // index of the message in messageDataList_
//      )
//    {
//        return messageItemList_.computeMessageBufferSize(messageDataList_[i]);
//    }
//
// //------------------------------------------------------------------------------------------------
//    void
//    MessageHandler::
//    writeBuffer
//    {
//        messageItemList_.write(pBuffer);
//    }
//
// //------------------------------------------------------------------------------------------------
//    void
//    MessageHandler::
//    readBuffer( void* pBuffer ) const
//    {
//        messageItemList_.read(pBuffer);
//    }

 //------------------------------------------------------------------------------------------------
 //------------------------------------------------------------------------------------------------
    INFO_DEF(MessageHandler)
    {
        std::stringstream ss;
        std::string t = title;
        if( !t.empty() ) t += ", ";
        t += tostr("key=", key_);
        ss<<indent<<"MessageHandler.info("<<t<<") :"
                  <<messageItemList().info(indent + "  ")
          <<indent<<"  messageDataList_ :"
          ;

        for( size_t m = 0; m < messageDataList_.size(); ++m ) {
            ss<<messageDataList_[m]->info(indent + "    ", tostr("message ", m, " of ",messageDataList_.size()));
        }
        return ss.str();
    }

 //------------------------------------------------------------------------------------------------
    STATIC_INFO_DEF(MessageHandler)
    {
        std::stringstream ss;
        ss<<"\nMessageHandler::static_info("<<title<<") :"
          <<theMessageHandlerRegistry.info(indent + "  ")
          <<Buffer::static_info(indent + "  ")
          ;

        return ss.str();
    }

//------------------------------------------------------------------------------------------------
}// namespace mpi