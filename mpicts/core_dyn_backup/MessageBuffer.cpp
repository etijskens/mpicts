#include "MessageBuffer.h"

namespace mpi
{//------------------------------------------------------------------------------------------------
 // Implementation of class MessageBuffer
 //-------------------------------------------------------------------------------------------------
    void
    MessageBuffer::
    allocate(size_t nBytes) // allocate a buffer that can accomodate a message of nBytes bytes.
                            // if the buffer is already large enough, it is reused (no-op)
    {
        size_t nWords = padBytes<sizeof(Word_t),sizeof(Word_t)>(nBytes);
         // now nBytes <= nWords * sizeof(Word_t) > nBytes + sizeof(Word_t)
        if( nWords <= size_ )
        {// buffer is large enough, nothing to do.
        } else
        {// current memory allocation is too small. Release it:
            free();
         // allocate large enough memory space.
            pBuffer_ = new Word_t[nWords];
            size_ = nWords;
        }
    }

 //-------------------------------------------------------------------------------------------------
    void
    MessageBuffer::
    free() // free the allocated memory, if any.
    {
        if( pBuffer_ ) {
            delete[] pBuffer_;
            pBuffer_ = nullptr;
            size_ = 0;
        }
    }

}// namespace mpi
