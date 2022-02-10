#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H


#include "mpicts.h"
#include "MessageItemList.h"

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


        template<typename MessageHandler>
        MessageHandlerBase&
        create()
        {
            MessageHandlerBase* pMessageHandler = new MessageHandler();
            registerMessageHandler(pMessageHandler);
            return *pMessageHandler;
        }

    private:
        size_t counter_;
        std::map<key_type,MessageHandlerBase*> registry_;
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
    protected: // data
        static bool const _debug_ = true;
        mutable MessageItemList messageItemList_;
        MessageHandlerRegistry::key_type key_;

    private: // ctor only to be used by MessageHandlerRegistry
        MessageHandlerBase();

    public:
     // data member access
        inline MessageItemList& message() { return messageItemList_; }
        inline MessageHandlerRegistry::key_type key() const { return key_; }

     // Member functions for MessageBuffer manipulation
        size_t computeBufferSize() { return messageItemList_.computeBufferSize();  }

        virtual ~MessageHandlerBase();

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
