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
     // This has to happen somewhere, admittedly this is not the most intuitive location for it.
     //
        if( MessageHeader::theHeaders.size() == 0 )
        {// Make sure that there is a MessageHeaderContainer for every MPI rank
            if constexpr(mpi::_debug_&&_debug_) {
                prdbg("MessageHeader::theHeaders.resize( (size_t) mpi::size );");
            }
            MessageHeader::theHeaders.resize( (size_t) mpi::size );
        }

        theMessageHandlerRegistry.registerMessageHandler(this);
    }

    MessageHandler&
    MessageHandler::
    create()
    {
        MessageHandler* pMessageHandler = new MessageHandler();
        return *pMessageHandler;
    }


    MessageHandler::
    ~MessageHandler()
    {
        if constexpr(::mpi::_debug_ && _debug_) prdbg( "~MessageHandler()" );

     // destroy the MessageData objects in sendMessages_:
        for( auto pMessageData : sendMessages_ )
        {
            if( pMessageData ) {
                if constexpr(::mpi::_debug_ && _debug_) prdbg(concatenate("delete pMessageData=", pMessageData));
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
            if constexpr(mpi::_debug_&&_debug_) {
                prdbg( concatenate( pMessageData->info("\n","MessageHandler::sendMessages(): buffer allocated")
                ));
            }

         // write the message to the buffer
            messageItemList().write(pMessageData);
            if constexpr(mpi::_debug_&&_debug_) {
                prdbg( concatenate( pMessageData->info("\n","MessageHandler::sendMessages(): message written to buffer")
                ));
            }

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
                if constexpr(mpi::_debug_&&_debug_) {
                    prdbg( concatenate( pMessageData->info("\n","MessageHandler::sendMessages(): message sent:")
                                , "\n  MPI_Isend(\n    "
                                , pMessageData->bufferPtr(), "\n    "   // pointer to buffer to send
                                , "nbytes=", pMessageData->size(), "\n    "       // number of Index_t elements to send
                                , "MPI_CHAR\n    "
                                , "dst=", pMessageData->dst(), "\n    "        // the destination
                                , "tag=", pMessageData->key(), "\n    "        // the tag
                                , "MPI_COMM_WORLD\n    &request"
                                , "\n  );\n"
                    ));
                }
             }
        }
    }

 //------------------------------------------------------------------------------------------------
    void
    MessageHandler::
    recvMessages() // Read the messages from the receive buffers
    {
        if constexpr(mpi::_debug_&&_debug_) {
            prdbg( concatenate( static_info("\n", "MessageHandler::recvMessages() entering")
            ));
        }

        for( auto pMessageData : recvMessages_ )
        {// allocate buffer for this message
            pMessageData->allocateBuffer();
            if constexpr(mpi::_debug_&&_debug_) {
                prdbg( concatenate( pMessageData->info("\n", "MessageHandler::recvMessages() buffer allocated")
                ));
            }

         // Receive the message
            if constexpr(mpi::_debug_&&_debug_) {
                prdbg( concatenate( pMessageData->info("\n", "MessageHandler::recvMessages() receiving message ")
                             , "\n  MPI_Recv("
                             , "\n    ", pMessageData->bufferPtr() // pointer to buffer where to store the message
                             , "\n    nBytes=", pMessageData->size()      // number of elements to receive
                             , "\n    MPI_CHAR"
                             , "\n    src=", pMessageData->src()       // source rank
                             , "\n    tag=", pMessageData->key()       // tag
                             , "\n    MPI_COMM_WORLD"
                             , "\n    MPI_STATUS_IGNORE"
                             , "\n  );"
                ));
            }
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
            if constexpr(mpi::_debug_&&_debug_) {
                prdbg( concatenate( pMessageData->info("\n", "MessageHandler::recvMessages() reading message from buffer")
                ));
            }
            messageItemList().read(pMessageData);
            if constexpr(mpi::_debug_&&_debug_) {
                prdbg( concatenate( pMessageData->info("\n", "MessageHandler::recvMessages() done")
                ));
            }
        }
    }

 //------------------------------------------------------------------------------------------------
    void MessageHandler::sendAllMessages()
    {
        for( auto & item : theMessageHandlerRegistry.registry_ )
        {
            MessageHandler& hndlr = *(item.second);
            hndlr.sendMessages();
        }
    }

 //------------------------------------------------------------------------------------------------
    void MessageHandler::recvAllMessages()
    {
        for( auto & item : theMessageHandlerRegistry.registry_ )
        {
            MessageHandler& hndlr = *(item.second);
            hndlr.recvMessages();
        }
    }

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
                ss<<sendMessages_[m]->info(indent + "    ", concatenate("message ", m, " of ",sendMessages_.size()));
            }
        } else {
            ss<<indent<<"    ( empty )";
        }

        ss<<indent<<"  recvMessages_ :";
        if( recvMessages_.size()) {
            for( size_t m = 0; m < recvMessages_.size(); ++m ) {
                ss<<recvMessages_[m]->info(indent + "    ", concatenate("message ", m, " of ",recvMessages_.size()));
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