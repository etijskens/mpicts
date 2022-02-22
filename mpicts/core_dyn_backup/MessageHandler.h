#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H


#include "mpicts.h"
#include "MessageItemList.h"
#include "ExtraMessageData.h"

#include <map>

namespace mpi
{//------------------------------------------------------------------------------------------------
    class MessageHandlerBase; // forward declaration

 //------------------------------------------------------------------------------------------------
   class MessageHandlerRegistry
 //------------------------------------------------------------------------------------------------
    {
    public:
        typedef MessageHandlerKey_t key_type; // if this must be changed, do it in "mpicts.h".

        MessageHandlerRegistry();
        // ~MessageHandlerRegistry() {}

     // Create a MessageHandler, store it in the registry_, and return a pointer to it.
        void registerMessageHandler(MessageHandlerBase* messageHandler);

        inline MessageHandlerBase& operator[](key_type key) {
            return *registry_[key];
        }

        template<typename MessageHandler, class... Args>
        MessageHandler&
        create()
        {
            MessageHandler* pMessageHandler = new MessageHandler(Args...);
            registerMessageHandler(pMessageHandler);
            return *pMessageHandler;
        }

    private:
        size_t counter_;
        std::map<key_type, MessageHandlerBase*> registry_;
    };

 // A global MessageHandlerRegistry
 // Check out https://stackoverflow.com/questions/86582/singleton-how-should-it-be-used
 // for when to use singletons and how to implement them.
    extern MessageHandlerRegistry theMessageHandlerRegistry;

 //------------------------------------------------------------------------------------------------
    class MessageHandlerBase
 // Base class for message handlers
 //------------------------------------------------------------------------------------------------
    {
        friend class MessageHandlerRegistry;

        static bool const _debug_ = true;
    protected: // data
        MessageHandlerRegistry::key_type key_; // Identification key of the MessageHandler in the registry.
        std::shared_ptr<MessageItemList> pMessageItemList_;
        MessageBuffer  messageBuffer_;  //
        MessageHeader* pMessageHeader_; // to access message header properties from the message body.
         // Note that pMessageHeader_ points to an entry in the MessageSet's messageHeaders std::vector.

        MessageHandlerBase() // default ctor protected
          : pMessageItemList_(new MessageItemList)
        {}

    public:
        static void createNew
        MessageHandlerBase(MessageHandlerBase& mh) // copy ctor
          : pMessageItemList_(mh.MessageItemList)
        {}

        virtual ~MessageHandlerBase();

     // data member access
        inline MessageItemList& messageItemList() { return *messageItemList_; }
        inline MessageHandlerRegistry::key_type key() const { return key_; }

        void addMessage(int destination);

     // Member functions for MessageBuffer manipulation
        size_t // buffer size in number of bytes.
        computeBufferSize();

        void
        writeBuffer       // Write the message in the messageBuffer
          ( void* pBuffer // pointer to buffer which is assumed large enough
          ) const;

        void
        readBuffer       // Write the message in the messageBuffer
          ( void* pBuffer // pointer to buffer which is assumed large enough
          ) const;

    };
 //------------------------------------------------------------------------------------------------
}// namespace mpi

#endif // MESSAGEHANDLER_H
