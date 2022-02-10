#ifndef MessageSet_H
#define MessageSet_H

#include <vector>
#include "mpicts.h"
#include "MessageHandler.h"

namespace mpi
{//------------------------------------------------------------------------------------------------
    struct MessageHeader
 //------------------------------------------------------------------------------------------------
    {
        MessageHandlerKey_t messageHandlerKey;  // MessageHandler key of the message
        size_t size;     // message size in bytes
        int source;      // the source MPI rank of the message
        int destination; // the destination MPI rank of the message
    };

 //------------------------------------------------------------------------------------------------
    class MessageSet;
 //------------------------------------------------------------------------------------------------
    class Message
 //------------------------------------------------------------------------------------------------
    {
        friend class MessageSet;
    private:
        MessageBuffer  messageBuffer_;  //
        MessageHeader* pMessageHeader_; // to access message header properties from the message body.
         // Note that pMessageHeader_ points to an entry in the MessageSet's messageHeaders std::vector.
    public:
     // read access member functions
        void* messageBuffer() const { return messageBuffer_.data(); }
        int destination()     const { return pMessageHeader_->destination; }
        int source()          const { return pMessageHeader_->source; }
        Index_t bufferSize()  const { return pMessageHeader_->size; }
        MessageHandlerKey_t
            messageHandlerKey()  const { return pMessageHeader_->messageHandlerKey; }
        MessageHandlerBase& messageHandler() const;

     // write access member functions
        void setDestination(int destination) { pMessageHeader_->destination = destination; }
        void setSource(int source)           { pMessageHeader_->source = source; }
        void setMessageHandlerKey(MessageHandlerKey_t key)
                                             { pMessageHeader_->messageHandlerKey = key; }

     // functionality
        void computeBufferSize();
        void adjustBuffer(); // adjust the MessageBuffer to the size in the MessageHeader.
        void writeBuffer();  // write the message to the buffer
        void sendBuffer();   // send the buffer to its destination MPI rank.
        void recvBuffer();   // receive the buffer
        void readBuffer();   // decode the buffer
    };
 //------------------------------------------------------------------------------------------------
    MessageHandlerBase& getMessageHandler(MessageHeader* pMessageHeader);
 //------------------------------------------------------------------------------------------------
    class MessageSet
 //------------------------------------------------------------------------------------------------
    {
        std::vector<MessageHeader> messageHeaders_;
        std::vector<Message>       messages_;
    public:
        void resize( size_t n ); // initialize n empty Messages.
        size_t size() const { return messages_.size(); }

        Message&
        addMessage      // add the message defined by `messageHandler` to the MessageSet
                        // This function is called by the sending process.
          ( int to_rank // destination MPI rank of the mesage
          , MessageHandlerBase& messageHandler
          );



     // machinery for iterating over the Messages
        typedef std::vector<Message>::iterator iterator;
        iterator begin() { return messages_.begin(); }
        iterator end()   { return messages_.end(); }

     //
        void send();
    };
 //------------------------------------------------------------------------------------------------
}// namespace mpi

#endif // MessageSet_H
