#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H


#include "mpicts.h"
#include "MessageItemList.h"
#include "MessageBuffer.h"

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

        template<typename MessageHandler, class... Args>
        MessageHandler&
        create(Args &&...args )
        {
            MessageHandler* pMessageHandler = new MessageHandler(args...);
            registerMessageHandler(pMessageHandler);
            return *pMessageHandler;
        }

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
        static MessageBufferPool theMessageBufferPool;
    private:
        friend class MessageHandlerRegistry;

        static bool const _debug_ = true;

    protected: // member class
        class MessageData
         // Base struct for message data.
         // Typically, a derived MessageHandler will have its own derived MessageData with additional
         // message data. As the derived MessageHandler knows the derived MessageData_Base type is can
         // use dynamic_cast to cast from  MessageData_Base
         //------------------------------------------------------------------------------------------------
        {
            MessageHeader messageHeader_;
            SharedBuffer  messageBuffer_;
        public:
            MessageData
              ( int src                 // MPI source rank
              , int dst                 // MPI source rank
              , MessageHandlerKey_t key //
              )
              : messageHeader_(src,dst,key)
            {// messageBuffer_ remains empty, sofar.
            }
            void getBuffer(size_t nBytes) { messageBuffer_ = theMessageBufferPool.getBuffer(nBytes); }
        };

    private: // data
        std::vector<MessageData*> messageDataList_;

    protected: // data
        mutable MessageItemList messageItemList_;
        key_type key_; // Identification key of the MessageHandler in the registry.

        MessageHandler() // default ctor protected
        {}

    public:
     // copy ctor
//        MessageHandler(MessageHandler const & messageHandler)
//          : key_             (messageHandler.key_)
//          , pMessageItemList_(messageHandler.pMessageItemList_)
//        {}

        virtual ~MessageHandler();

     // data member access
        inline MessageItemList& messageItemList() {
            return messageItemList_;
        }

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
