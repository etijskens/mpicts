#ifndef MESSAGEHEADER_H
#define MESSAGEHEADER_H

#include "mpicts.h"

namespace mpi
{//------------------------------------------------------------------------------------------------
    class MessageHeader
    {
//        inline Index_t messageBegin       (Index_t msgid) const { return pBuffer_[1 + HEADER_SIZE * msgid + MSG_BGN]; }
//        inline Index_t messageEnd         (Index_t msgid) const { return pBuffer_[1 + HEADER_SIZE * msgid + MSG_END]; }
//        inline int     messageDestination (Index_t msgid) const { return pBuffer_[1 + HEADER_SIZE * msgid + MSG_DST]; }
//        inline int     messageSource      (Index_t msgid) const { return pBuffer_[1 + HEADER_SIZE * msgid + MSG_SRC]; }
//        inline Index_t messageHandlerKey  (Index_t msgid) const { return pBuffer_[1 + HEADER_SIZE * msgid + MSG_KEY]; }
          int dst_;             // the destination MPI rank of the message
          int src_;             // the source MPI rank of the message
          Index_t sz_;          // message size (??? in bytes or number of Index_t elements???)
          Index_t msgHndlrKey_; // MessageHandler key of the message
    public:
        MessageHeader
          ( int src             // source MPI rank of the message
          , int dst             // destination MPI rank of the message
          , Index_t msgHndlrKey // MessageHandler key of the message
          , Index_t sz=0        // message size
          );


    private:
    };
 //------------------------------------------------------------------------------------------------

}// namespace mpi

#endif // MESSAGEHEADER_H
