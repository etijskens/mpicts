#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H


#include "mpicts.h"
#include "MessageItemList.h"
#include "MessageData.h"

#include <map>

namespace mpi
{//------------------------------------------------------------------------------------------------
    class MessageHandler; // forward declaration

 //------------------------------------------------------------------------------------------------
   class MessageHandlerRegistry
 // Lookup MessageHandlers from their key.
 // MessageHandlers are created using new and automatically added to the MessageHandlerRegistry
 // which takes ownership of the MessageHandler

 // TODO: currently all MessageHandlers are kept for the entire time of the simulation
 //   - how about MessageHandlers that act only once, or once and a while?
 //------------------------------------------------------------------------------------------------
    {
        friend class MessageHandler;
    public:
        using Key_t = MessageHandlerKey_t; // if this must be changed, do it in "mpicts.h".

        ~MessageHandlerRegistry();

     // Create a MessageHandler, store it in the registry_, and return a pointer to it.
        void registerMessageHandler(MessageHandler* messageHandler);

        inline MessageHandler& operator[](Key_t key) {
            return *registry_[key];
        }

        INFO_DECL;

    private:
        Key_t generateKey_();

        std::map<Key_t, MessageHandler*> registry_;
    };

 //------------------------------------------------------------------------------------------------
    class MessageHandler
 // Base class for message handlers
 // Every MessageHandler has a single MessageItemList which identifies the objects from whom which
 // data are sent, and in which data are received.
 // A MessageHandler may send/receive any number of messages with information from the objects in
 // the MessageItems. There is a list of messages to send (sendMessages_) and a list of messages
 // to receive (recvMessages_). The sendMessages_ must be created in the sending process. The
 // recvMessages_ are automatically created by the receiving processes from the MessageHeaders
 // which are broadcasted.
 // A MessageHandler may send
 //   - messages to any number of destination processes
 //   - more than one message to the same destination process (these are disambiguated using the
 //     MPI tag)
 //------------------------------------------------------------------------------------------------
    {
    public:
        using Key_t = MessageHandlerRegistry::Key_t;
        static MessageHandlerRegistry theMessageHandlerRegistry;

        static bool const _debug_ = true;

        friend class MessageHandlerRegistry;

    protected: // member class

    protected: // data
        std::vector<MessageData*> sendMessages_; // one entry for each message to send using this MessageHandler's messageItemList_
        std::vector<MessageData*> recvMessages_; // one entry for each message to receive using this MessageHandler's messageItemList_

    protected: // data
        mutable MessageItemList messageItemList_; // the entries reference the objects from which the message is composed
        Key_t key_; // Identification key of the MessageHandler in the registry.

        MessageHandler();
    public:
        static MessageHandler& create();

        virtual ~MessageHandler();

     // data member access
        inline MessageItemList const& messageItemList() const { return messageItemList_; }
        inline MessageItemList      & messageItemList()       { return messageItemList_; }

        inline MessageHandlerRegistry::Key_t key() const { return key_; }

        INFO_DECL;
        STATIC_INFO_DECL;

     // High level member functions for MessageBuffer manipulation
        virtual void addSendMessage(int destination);
        virtual void addRecvMessage(int src, size_t i);

        inline size_t nSendMessages() const { return sendMessages_.size(); }
        inline size_t nRecvMessages() const { return recvMessages_.size(); }

        void computeMessageBufferSizes();
         // compute the size of all messages this MPI rank wil send, and store it in its MessageHeader.

        void sendMessages();
         // Allocate buffers, write the messages ito the buffers, and send them.
         // (sends only the messages from this MessageHandler)

        void recvMessages();
         // receive the messages in the receive buffers, and read them into their objects
         // (receives only the messages for this MessageHandler)

        static void sendAllMessages(); // Send all message from all registered MessageHandlers
        static void recvAllMessages(); // Receive all message for all registered MessageHandlers
    };
 //------------------------------------------------------------------------------------------------
}// namespace mpi

#endif // MESSAGEHANDLER_H
