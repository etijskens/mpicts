#include "MessageSet.h"

namespace mpi
{//------------------------------------------------------------------------------------------------
 // implementation of class Message
 //------------------------------------------------------------------------------------------------
    MessageHandlerBase&
    Message::
    messageHandler() const
    {
        MessageHandlerKey_t key = pMessageHeader_->messageHandlerKey;
        return theMessageHandlerRegistry[key];
    }

    void
    Message::
    computeBufferSize()
    {
        pMessageHeader_->size = messageHandler().computeBufferSize();
    }


    void
    Message::
    adjustBuffer() // adjust the MessageBuffer to the size in the MessageHeader.
    {
        size_t messageSize = pMessageHeader_->size;
        messageBuffer_.allocate(messageSize);
    }

    void
    Message::
    writeBuffer() // write the message to the buffer
    {
        void* pBuffer = this->messageBuffer();
        this->messageHandler().writeBuffer(pBuffer);
    }

    void
    Message::
    sendBuffer() // send the buffer to its destination (non-blocking)
    {
        MPI_Request request;
        int success =
        MPI_Isend                       // non-blocking
          ( this->messageBuffer()
          , this->bufferSize()
          , MPI_LONG_LONG_INT
          , this->destination()
          , this->messageHandlerKey()   // tag
          , MPI_COMM_WORLD
          , &request
          );
    }

    void
    Message::
    recvBuffer()
    {
        int succes =
        MPI_Recv                        // blocking
          ( this->messageBuffer()
          , this->bufferSize()
          , MPI_LONG_LONG_INT
          , this->source()
          , this->messageHandlerKey()   // tag
          , MPI_COMM_WORLD
          , MPI_STATUS_IGNORE
          );
    }

    void
    Message::
    readBuffer()
    {
        void* pBuffer = this->messageBuffer();
        this->messageHandler().readBuffer(pBuffer);
    }

 //------------------------------------------------------------------------------------------------
 // implementation of class MessageSet
 //------------------------------------------------------------------------------------------------
    void
    MessageSet::
    resize( size_t n )
    {
        messageHeaders_.resize(n);
        messages_      .resize(n);
        for( size_t i=0; i < n; ++i )
        {
            MessageHeader& header = messageHeaders_[i];
            header.destination           = -1;
            header.source                = -1;
            header.messageHandlerKey     = -1;
            header.size                  =  0;
            messages_[i].pMessageHeader_ = &header;
        }
    }
 //------------------------------------------------------------------------------------------------
    Message&      // index of the added message
    MessageSet::
    addMessage  // add the message defined by messageHandler to the MessageSet
      ( int to_rank
      , MessageHandlerBase& messageHandler
      )
    {
        size_t nMessages = messages_.size();
        messages_      .resize(nMessages+1);
        messageHeaders_.resize(nMessages+1);

        Message& message = messages_[nMessages];
        message.pMessageHeader_ = &messageHeaders_[nMessages];

        message.setDestination( to_rank );
        message.setSource( mpi::rank );
        message.setMessageHandlerKey( messageHandler.key() );
     // message.computeBufferSize();
     //     It is safer to compute buffer size before the buffer is written, otherwise we must hope thet the user
     //     does not add messageItems after calling addMessage. Instead, we initialize the buffer's size to 0.
        message.pMessageHeader_->size = 0;

        return message;
    }

 //------------------------------------------------------------------------------------------------
}// namespace mpi