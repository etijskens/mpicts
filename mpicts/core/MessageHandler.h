#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H


#include "mpicts.h"
#include "Message.h"

#include <map>

namespace mpi
{//------------------------------------------------------------------------------------------------
    class MessageHandlerBase; // forward declaration

 //------------------------------------------------------------------------------------------------
   class MessageHandlerRegistry
 //------------------------------------------------------------------------------------------------
    {
    public:
        typedef MessageHandlerKey_t key_type; // if this must be changed, do it in "types.h".

        MessageHandlerRegistry();
        // ~MessageHandlerRegistry() {}

     // Create a MessageHandler, store it in the registry_, and return a pointer to it.
        void registerMessageHandler(MessageHandlerBase* messageHandler);

        inline MessageHandlerBase& operator[](key_type key) {
            return *registry_[key];
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
    protected:
       static bool const _debug_ = true;
    public:
        typedef MessageHandlerRegistry::key_type key_type;

     // the MessageHandler typically lives as long as a simulation. It is practical to store a
     // reference to it.
        MessageHandlerBase();
        virtual ~MessageHandlerBase();

     // Post the message in the messageBuffer
     //     message -> messageBuffer
        Index_t // message id of the message written. Occasionally useful.
        writeMessage
          ( int for_rank // destination of the message (some MPI rank).
          );

     // Read a message in the messageBuffer, i.e. the inverse of postMessagge().
     //     message <- messageBuffer
     // Reading the message is generally the responsability of the messageBuffer, which
     //  - first retrieves the MessageHandlerKey from a message header
     //  - then looks up the corresponding MessageHandler in theMessageHandlerRegistry
     //  - finally delegates the work to MessageHandler::readMessage()
     // Returns true if the MessageHandlerKey value in the message header
     // with id msg_id corresponds to this->key_, false otherwise.
     // in which case the message was not written by this
     // MessageHandler, and therefore also cannot be read by it.
        virtual
        bool               // succes indicator
        readMessage
          ( Index_t msg_id // the message id identifies the message header
          );

//        virtual
//        bool
//        readMessage
//          ( Index_t msg_id // the message id identifies the message header
//          , Indices_t indices
//          );

     // data member access
        inline Message& message() { return message_; }
        inline key_type key() const { return key_; }

    protected:
        Message message_;
        key_type key_;
    };
 //------------------------------------------------------------------------------------------------
}// namespace mpi

#endif // MESSAGEHANDLER_H
