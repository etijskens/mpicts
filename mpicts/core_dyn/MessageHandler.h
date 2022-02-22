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
 //------------------------------------------------------------------------------------------------
    {
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
    private:
        friend class MessageHandlerRegistry;

        static bool const _debug_ = true;

    protected: // member class

    private: // data
        std::vector<MessageData*> messageDataList_; // one entry for each message using this MessageHandler's messageItemList_

    protected: // data
        mutable MessageItemList messageItemList_; // the entries reference the objects from which the message is composed
        key_type key_; // Identification key of the MessageHandler in the registry.

    public:
        MessageHandler();

        virtual ~MessageHandler();

     // data member access
        inline MessageItemList const& messageItemList() const { return messageItemList_; }
        inline MessageItemList      & messageItemList()       { return messageItemList_; }

        inline MessageHandlerRegistry::key_type key() const { return key_; }

        INFO_DECL;
        STATIC_INFO_DECL;

     // High level member functions for MessageBuffer manipulation
        void addMessage(int destination);

        inline size_t nMessages() const { return messageDataList_.size(); }

        void postMessages();     // Allocate buffers and write the messages to their buffers.
        void transferMessages(); // Send and receive the messages
        void readMessages();     // Read the messages from the receive buffers

    protected:
     // Low level member functions for MessageBuffer manipulation
//        size_t                   // number of bytes the message needs
//        computeMessageBufferSize // Compute the size (bytes) that a message will occupy when written to a buffer
//          ( size_t i             // index of the message in messageDataList_
//          );
//
//        void
//        writeBuffer   // Write the i-th message to its messageBuffer. The buffer is automatically adjusted.
//          ( size_t i  // index of the message in messageDataList_
//          );
//
//        void
//        readBuffer    // Read the i-th message from the messageBuffer. The buffer is automatically adjusted.
//          ( size_t i  // index of the message in messageDataList_
//          );
    };
 //------------------------------------------------------------------------------------------------
}// namespace mpi

#endif // MESSAGEHANDLER_H
