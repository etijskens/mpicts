#ifndef MESSAGEDATA_H
#define MESSAGEDATA_H

#include "MessageHeader.h"
#include "MessageBuffer.h"

namespace mpi
{//------------------------------------------------------------------------------------------------
    class MessageData
 // Base struct for message data.
 // Typically, a derived MessageHandler will extend MessageData. E.g. A ParticleContainerMessageHandler
 // will add the list of selected particles, and a bool indicating whether those particles need to be
 // moved or copied.
 //------------------------------------------------------------------------------------------------
    {
    protected:
        MessageHeader messageHeader_;
        MessageBuffer messageBuffer_;
    public:
        using Key_t = MessageHeader::Key_t;

     // ctor
        MessageData   // Create MessageData for sending a message
          ( int src   // MPI source rank
          , int dst   // MPI destination rank
          , Key_t key // MessageHandler key
          )
          : messageHeader_(src,dst,key)
        {// messageBuffer_ remains empty, sofar.
        }

        MessageData  // Create MessageData for receiving a message
          ( int src  // MPI source rank from whom to receive
          , size_t i // location in MessageHeaderContainer for MPI rank src
          )
          : messageHeader_(src, i)
        {
            allocateBuffer();
        }

        virtual ~MessageData() {}

        void allocateBuffer();


        Key_t    key()  const { return messageHeader_.key(); }
        MPITag_t tag()  const { return messageHeader_.tag(); }
        int      src()  const { return messageHeader_.src(); }
        int      dst()  const { return messageHeader_.dst(); }
        size_t   size() const { return messageHeader_.size(); }
        size_t&  size()       { return messageHeader_.size(); } // the size of the message

        void*   bufferPtr()  const { return messageBuffer_.ptr(); }
        size_t  bufferSize() const { return messageBuffer_.size(); } // the size of the buffer, >= the size of the message

        virtual INFO_DECL;
    };
 //------------------------------------------------------------------------------------------------
}// namespace mpi

#endif // MESSAGEDATA_H
