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
 // TODO: currently all MessageHandlers are kept for the entire time of the simulation
 //   - how about MessageHandlers that act only once, or once and a while?
 //------------------------------------------------------------------------------------------------
    {
        friend class MessageHandler;
    public:
        using key_type = MessageHandlerKey_t; // if this must be changed, do it in "mpicts.h".

        MessageHandlerRegistry();
        // ~MessageHandlerRegistry() {}

     // Create a MessageHandler, store it in the registry_, and return a pointer to it.
        void registerMessageHandler(MessageHandler* messageHandler);

        inline MessageHandler& operator[](key_type key) {
            return *registry_[key];
        }

        INFO_DECL;
    private:
        size_t counter_;
        std::map<key_type, MessageHandler*> registry_;
    };

 // A global MessageHandlerRegistry
 // Check out https://stackoverflow.com/questions/86582/singleton-how-should-it-be-used
 // for when to use singletons and how to implement them.
 // extern MessageHandlerRegistry theMessageHandlerRegistry;

 //------------------------------------------------------------------------------------------------
    class MessageHandler
 // Base class for message handlers
 //------------------------------------------------------------------------------------------------
    {
    public:
        using key_type = MessageHandlerRegistry::key_type;
        static MessageHandlerRegistry theMessageHandlerRegistry;

        static bool const _debug_ = true;

        friend class MessageHandlerRegistry;

    protected: // member class

    protected: // data
        std::vector<MessageData*> sendMessages_; // one entry for each message to send using this MessageHandler's messageItemList_
        std::vector<MessageData*> recvMessages_; // one entry for each message to receive using this MessageHandler's messageItemList_

    protected: // data
        mutable MessageItemList messageItemList_; // the entries reference the objects from which the message is composed
        key_type key_; // Identification key of the MessageHandler in the registry.

        MessageHandler();
    public:
        static MessageHandler& create();

        virtual ~MessageHandler();

     // data member access
        inline MessageItemList const& messageItemList() const { return messageItemList_; }
        inline MessageItemList      & messageItemList()       { return messageItemList_; }

        inline MessageHandlerRegistry::key_type key() const { return key_; }

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
