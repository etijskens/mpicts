#ifndef MESSAGEBUFFER_H
#define MESSAGEBUFFER_H

#include "mpicts.h"

#include <vector>

namespace mpi
{//------------------------------------------------------------------------------------------------
 // padBytes function
 //------------------------------------------------------------------------------------------------
//    template
//      < size_t WordSize // Pad the nBytes parameter to the next WordSize boundary
//      , size_t Unit=1   // Express the result as a multiple of Unit bytes (Unit must be a divisor of WordSize).
//      >
//    size_t              // the result
//    padBytes            // padBytes<8>(5)   -> 8 : the next 8 byte boundary of 5 (bytes) is 8 (bytes)
//                        // padBytes<8,4>(5) -> 2 : the next 8 byte boundary of 5 (bytes) is 2 4-byte words (= 8 bytes)
//                        // padBytes<8,2>(5) -> 8 : the next 8 byte boundary of 5 (bytes) is 1 8-byte word (= 8 bytes)
//      ( size_t nBytes   // the number of bytes to be padded.
//      )
//    {
//        static_assert(WordSize % Unit == 0); // Unit must be a divisor of WordSize.
//        return ((nBytes + WordSize - 1) / WordSize) * (WordSize / Unit);
//    }

 //------------------------------------------------------------------------------------------------
    class MessageBuffer
 //------------------------------------------------------------------------------------------------
    {
        size_t nBytes_;
        char* pBuffer_; // pointer to the beginning of the buffer
    public:
        MessageBuffer()
          : nBytes_(0)
          , pBuffer_( nullptr )
        {}

        void alloc(size_t nBytes)
        {
            if( nBytes > nBytes_) {
                free();
                pBuffer_ = new char[nBytes];
                nBytes_ = nBytes;
            } else
            {// buffer is larger than needed but that doesn't harm.
            }
        }

        void free()
        {
            if( pBuffer_ ) {
                delete[] pBuffer_;
                nBytes_ = 0;
            }
        }

        ~MessageBuffer()
        {
            free();
        }

        void*  ptr() const { return pBuffer_; }
      //void*  ptr()       { return pBuffer_; }
        size_t size() const { return nBytes_; }
      //size_t size()       { return nBytes_; }
        INFO_DECL;
    };

 //------------------------------------------------------------------------------------------------
}// namespace mpi

#endif // MESSAGEBUFFER_H
