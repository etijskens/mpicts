#ifndef MESSAGEBUFFER_H
#define MESSAGEBUFFER_H

#include <vector>

namespace mpi
{//------------------------------------------------------------------------------------------------
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
    class Buffer
 //------------------------------------------------------------------------------------------------
    {
        friend class SharedBuffer;
        friend class MessageBufferPool;
    private:
        size_t nBytes_; // number of words (Word_t) allocated.
        char*  pBytes_; // pointer to the buffer
        unsigned int useCount_;

    public:
        Buffer(size_t nBytes)
          : nBytes_( nBytes )
          , pBytes_( new char[nBytes_] )
          , useCount_( 0 )
        {}

     // copy ctor that actually moves the resources from the buffer being copied.
        Buffer( Buffer const& buffer)
          : nBytes_(buffer.nBytes_)
          , pBytes_(buffer.pBytes_)
          , useCount_(buffer.useCount_)
        {
            Buffer& non_const_buffer = const_cast<Buffer&>(buffer);
            non_const_buffer.nBytes_ = 0;
            non_const_buffer.pBytes_ = nullptr; // this avoids destroying the raw buffer when the Buffer object is copied
            non_const_buffer.useCount_ = 0;
        }

        ~Buffer()
        {
            if( pBytes_ ) {
                //std::cout<<"\n~Buffer(): delete[] "<<pBytes_<<std::flush;
                delete[] pBytes_;
                //std::cout<<", deleted."<<std::endl;
            } else {
                //std::cout<<"~Buffer() with nullptr"<<std::endl;
            }
        }

        size_t size() const { return nBytes_; }

        void* raw() { return pBytes_; }

        std::string info( std::string const& s = std::string() ) const;
    };

 //------------------------------------------------------------------------------------------------
    class SharedBuffer
 //------------------------------------------------------------------------------------------------
    {
        Buffer* pBuffer_;
    public:
        SharedBuffer()
          : pBuffer_( nullptr )
        {}

        SharedBuffer(Buffer* pBuffer)
          : pBuffer_( pBuffer )
        {
            ++(pBuffer_->useCount_);
        }

     // Copy assignment moves the Buffer* from the rhs to *this (transferring ownership)
        SharedBuffer& operator=(const SharedBuffer& rhs)
        {
            std::cout<<"coucou"<<std::endl;
            if (this == &rhs) return *this; // Guard self assignment

         // forget the current pBuffer_ (if any) and move the one from the rhs to *this
            if( pBuffer_ ) --(pBuffer_->useCount_);
            pBuffer_ = rhs.pBuffer_;
            SharedBuffer& non_const_rhs = const_cast<SharedBuffer&>(rhs);
            non_const_rhs.pBuffer_ = nullptr;

            return *this;
        }

        ~SharedBuffer()
        {
            if( pBuffer_ )
                --pBuffer_->useCount_;
        }

        void* raw() { return pBuffer_->raw(); }

        std::string info( std::string const& s = std::string() ) const;
    };

 //------------------------------------------------------------------------------------------------
    class MessageBufferPool
 //-------------------------------------------------------------------------------------------------
    {
    private: // data members
        std::vector<Buffer> pool_;

    public:
        SharedBuffer
        getBuffer       // Get a buffer from the pool
          ( size_t size // the minimum size of the buffer in bytes
          );

        std::string info( std::string const& s = std::string() ) const;
    };
 //------------------------------------------------------------------------------------------------
}// namespace mpi

#endif // MESSAGEBUFFER_H
