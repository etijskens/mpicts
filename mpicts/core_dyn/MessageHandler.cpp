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
     // This has to happen somewhere
        if( MessageHeader::theHeaders.size() == 0 )
        {// Make sure that there is a MessageHeaderContainer for every MPI rank
            prdbg("MessageHeader::theHeaders.resize( (size_t) mpi::size );");
            MessageHeader::theHeaders.resize( (size_t) mpi::size );
        }

        theMessageHandlerRegistry.registerMessageHandler(this);
    }

    MessageHandler::
    ~MessageHandler()
    {
        if constexpr(::mpi::_debug_ && _debug_) prdbg( "~MessageHandler()" );

     // destroy the MessageData objects in sendMessages_:
        for( auto pMessageData : sendMessages_ )
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
    addSendMessage(int destination)
    {
        assert( destination < mpi::size
             && "Invalid MPI rank for destination."
              ); // https://stackoverflow.com/questions/3692954/add-custom-messages-in-assert/26984456
         // We allow sending to self rank for testing.

        sendMessages_.push_back(new MessageData( mpi::rank, destination, this->key_ ));
    }

 //------------------------------------------------------------------------------------------------
    void
    MessageHandler::
    addRecvMessage(int src, size_t i)
    {
        recvMessages_.push_back(new MessageData(src, i));
    }

 //------------------------------------------------------------------------------------------------
    void
    MessageHandler::
    computeMessageBufferSizes()
    {
        for( auto pMessageData : sendMessages_ ) {
            messageItemList().computeMessageBufferSize(pMessageData);
        }
    }

 //------------------------------------------------------------------------------------------------
    void
    MessageHandler::
    sendMessages() // Allocate buffers and write the messages to their buffers.
    {// Note that MessageHeader::broadcastMessageHeaders() must be called before calling
     // sendMessages(), to make sure that every rank knows all the MessageHeaders

        for( auto pMessageData : sendMessages_ )
        {// allocate buffer for this message
            pMessageData->allocateBuffer();
            prdbg(pMessageData->info("\n","sendMessages"));

         // write the message to the buffer
            messageItemList().write(pMessageData);

         // send the message
            if( mpi::size > 1 )
            {
                MPI_Request request;
                int success =
                MPI_Isend                       // non-blocking send
                  ( pMessageData->bufferPtr()   // pointer to buffer to send
                  , pMessageData->size()        // number of Index_t elements to send
                  , MPI_CHAR
                  , pMessageData->dst()         // the destination
                  , pMessageData->key()         // the tag
                  , MPI_COMM_WORLD
                  , &request
                  );
             // todo: We have a problem if a MessageHandler does more than one send with the same destination:
             // Then source and tag=key are no longer unique. This is probable happen for a
             // ParticleContainerMessageHandler sending ghost particles and leaving particles in a separate go.
             // One solution is to select a random tag and put it into the MessageHeader, so that the receiver
             // can pick up the tag there.
            }
        }
    }

 //------------------------------------------------------------------------------------------------
    void
    MessageHandler::
    recvMessages() // Read the messages from the receive buffers
    {
        prdbg( tostr(static_info("\n", "recvMessages()")
                    )
             );
        for( auto pMessageData : recvMessages_ )
        {// allocate buffer for this message
            prdbg(pMessageData->info("\n","recvMessages"));
            pMessageData->allocateBuffer();

         // Receive the message
            int succes =
            MPI_Recv
              ( pMessageData->bufferPtr() // pointer to buffer where to store the message
              , pMessageData->size()      // number of elements to receive
              , MPI_CHAR
              , pMessageData->src()       // source rank
              , pMessageData->key()       // tag
              , MPI_COMM_WORLD
              , MPI_STATUS_IGNORE
              );

         // read the message from the buffer
            messageItemList().read(pMessageData);
        }
    }

 //------------------------------------------------------------------------------------------------
//    size_t                   // number of bytes the message needs
//    MessageHandler::
//    computeMessageBufferSize // Compute the size (bytes) that a message will occupy when written to a buffer
//      ( size_t i             // index of the message in sendMessages_
//      )
//    {
//        return messageItemList_.computeMessageBufferSize(sendMessages_[i]);
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
        if( !title.empty() ) ss<<title;

        ss<<indent<<"MessageHandler.info("<<"key="<<key_<<") :"
                  <<messageItemList().info(indent + "  ")
          <<indent<<"  sendMessages_ :";
        if( sendMessages_.size()) {
            for( size_t m = 0; m < sendMessages_.size(); ++m ) {
                ss<<sendMessages_[m]->info(indent + "    ", tostr("message ", m, " of ",sendMessages_.size()));
            }
        } else {
            ss<<indent<<"    ( empty )";
        }

        ss<<indent<<"  recvMessages_ :";
        if( recvMessages_.size()) {
            for( size_t m = 0; m < recvMessages_.size(); ++m ) {
                ss<<recvMessages_[m]->info(indent + "    ", tostr("message ", m, " of ",recvMessages_.size()));
            }
        } else {
            ss<<indent<<"    ( empty )";
        }
        return ss.str();
    }

 //------------------------------------------------------------------------------------------------
    STATIC_INFO_DEF(MessageHandler)
    {
        std::stringstream ss;
        ss<<"\nMessageHandler::static_info("<<title<<") :"
          <<theMessageHandlerRegistry.info(indent + "  ")
          ;

        return ss.str();
    }

//------------------------------------------------------------------------------------------------
}// namespace mpi