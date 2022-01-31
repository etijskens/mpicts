#include "MessageHeader.h"

namespace mpi
{//------------------------------------------------------------------------------------------------
    MessageHeader::
    MessageHeader
      ( int src             // source MPI rank of the message
      , int dst             // destination MPI rank of the message
      , Index_t msgHndlrKey // MessageHandler key of the message
      , Index_t sz          // message size
      )
      : src_(src)
      , dst_(dst)
      , msgHndlrKey_(msgHndlrKey)
      , sz_(0)
    {}
 /*
    void
    MessageHeader::
    void
    MessageHeader::
    alloc(Index_t sz) // allocate memory for a message of size sz (or smaller)
    {
        free();
        msg_ptr_ = new Index_t[](sz);
        sz_ = sz;
    }

    void
    MessageHeader::
    free() // deallocate the memory for the message
    {
        if( msg_ptr_ ) {
            delete[] msg_ptr_;
            msg_ptr_ = nullptr;
            sz_ = 0;
        }
    }
 */
 //------------------------------------------------------------------------------------------------

}// namespace mpi