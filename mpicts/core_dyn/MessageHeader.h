#ifndef MESSAGEHEADER_H
#define MESSAGEHEADER_H

#include <vector>
#include "mpicts.h"
#define INVALID -1

namespace mpi
{//------------------------------------------------------------------------------------------------
    struct MessageHeaderData
 // Struct with the data needed for a MessageHeader
 //------------------------------------------------------------------------------------------------
    {
        MessageHandlerKey_t key;  // MessageHandler key of the message
        size_t size;     // message size in bytes
        int source;      // the source MPI rank of the message
        int destination; // the destination MPI rank of the message

        void invalidate() {
            source = INVALID;
        }

        bool isValid() const {
            return source != INVALID;
        }

        INFO_DECL;
    };

 //------------------------------------------------------------------------------------------------
    class MessageHeaderContainer
 //
 //------------------------------------------------------------------------------------------------
    {
        std::vector<MessageHeaderData> headers_;
         // MessageHeaderData only live inside as entries in a std::vector which is used as an MPI
         // buffer.
    public:
        static size_t nBytesPerHeader; // number of bytes per MessageHeaderData in the std::vector
         // This might differ from sizeof(MessageHeaderData) for alignment reasons.

    public:
        MessageHeaderContainer();

        size_t       // return the index of the added MessageHeaderData in headers_
        addHeader(); // add an empty message header

        MessageHeaderData& operator[] (size_t i) {
            return headers_[i];
        }

     // tools for using the headers_ as an MPI buffer

     // pointer to the header buffer (for sending)
        char* buffer() const {
            return (char*)(headers_.data());
        }

     // size of the buffer (= number of headers)
        size_t size() const {
            return headers_.size();
        }

     // resize the buffer (for receiving headers)
        void resize(size_t nHeaders) {
            headers_.resize(nHeaders);
        }

        INFO_DECL;

    private:
        void computeNBytesPerHeader_();
    };
    
 //------------------------------------------------------------------------------------------------
    class MessageHeader
 // Wrapper class for MessageHeaderData. The actual location of the data is an entry in a 
 // std::vector<MessageHeaderData>
 //------------------------------------------------------------------------------------------------
    {
        int src_; // source rank
        size_t i_; // location of the header in headers_
         // Indices are not invalidated when theHeaders is resized, as opposed to pointers and iterators.

    public: // data
        static std::vector<MessageHeaderContainer> theHeaders;
         // One MessageHeaderContainer per MPI rank


    public:
        MessageHeader
          ( int src                 // MPI source rank
          , int dst                 // MPI source rank
          , MessageHandlerKey_t key //
          , size_t sz = 0           // size of message in bytes, usually set later (must be computed first
          );

        MessageHeader
          ( MessageHeaderData* pMessageHeaderData
          );

     // Default copy ctor should be good to go

        int  source()              const { return theHeaders[src_][i_].source; }
        int& source()                    { return theHeaders[src_][i_].source; }
        int  destination()         const { return theHeaders[src_][i_].destination; }
        int& destination()               { return theHeaders[src_][i_].destination; }
        MessageHandlerKey_t  key() const { return theHeaders[src_][i_].key; }
        MessageHandlerKey_t& key()       { return theHeaders[src_][i_].key; }
        size_t  size()             const { return theHeaders[src_][i_].size; }
        size_t& size()                   { return theHeaders[src_][i_].size; }

        INFO_DECL;
        STATIC_INFO_DECL;

    private:
        void alloc_();
    };
    
 //------------------------------------------------------------------------------------------------
}// namespace mpi

#endif // MESSAGEHEADER_H
