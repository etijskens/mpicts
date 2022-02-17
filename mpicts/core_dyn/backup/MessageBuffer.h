#ifndef MESSAGEBUFFER_H
#define MESSAGEBUFFER_H

#include "mpicts.h"
#include "memcpy_able.h"

#include <vector>


namespace mpi
{//------------------------------------------------------------------------------------------------
    class MessageBuffer
 //-------------------------------------------------------------------------------------------------
    {
    private: // typedefs
        typedef Index_t Word_t; // The size of the buffer is internally expressed in terms of Word_t blocks.

    private: // data members
        Word_t * pBuffer_;
        size_t size_; // measured in number of Word_t words allocated.

    public:
        MessageBuffer() // Construct empty MessageBuffer object (without allocated memory).
          : pBuffer_(nullptr), size_(0)
        {}

        ~MessageBuffer()
        {
            free();
        }

        void* // void pointer to the buffer
        data() const
        {
            return static_cast<void*>(pBuffer_);
        }

        void allocate(size_t nBytes); // allocate a buffer that can accomodate a message of nBytes bytes.

        void free(); // free the allocated memory, if any.
    };
 //------------------------------------------------------------------------------------------------
 // padBytes function
 //------------------------------------------------------------------------------------------------
    template
      < size_t WordSize // Pad the nBytes parameter to the next WordSize boundary
      , size_t Unit=1   // Express the result as a multiple of Unit bytes (Unit must be a divisor of WordSize).
      >
    size_t              // the result
    padBytes            // padBytes<8>(5)   -> 8 : the next 8 byte boundary of 5 (bytes) is 8 (bytes)
                        // padBytes<8,4>(5) -> 2 : the next 8 byte boundary of 5 (bytes) is 2 4-byte words (= 8 bytes)
                        // padBytes<8,2>(5) -> 8 : the next 8 byte boundary of 5 (bytes) is 1 8-byte word (= 8 bytes)
      ( size_t nBytes   // the number of bytes to be padded.
      )
    {
        static_assert(WordSize % Unit == 0); // Unit must be a divisor of WordSize.
        return ((nBytes + WordSize - 1) / WordSize) * (WordSize / Unit);
    }
 //------------------------------------------------------------------------------------------------
}// namespace mpi

#endif // MESSAGEBUFFER_H
