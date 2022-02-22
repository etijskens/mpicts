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
        MessageHeader messageHeader_;
        MessageBuffer messageBuffer_;
    public:
     // ctor
        MessageData
          ( int src                 // MPI source rank
          , int dst                 // MPI destination rank
          , MessageHandlerKey_t key //
          )
          : messageHeader_(src,dst,key)
        {// messageBuffer_ remains empty, sofar.
        }

        void getBuffer(size_t nBytes);

        INFO_DECL;
    };
 //------------------------------------------------------------------------------------------------
}// namespace mpi

#endif // MESSAGEDATA_H
