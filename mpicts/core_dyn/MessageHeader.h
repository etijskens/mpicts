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
        using Key_t = MessageHandlerKey_t;

        Key_t    key;  // MessageHandler key of the message
        MPITag_t tag;  // MPI tag for the message
        size_t   size; // message size in bytes
        int      src;  // the source MPI rank of the message
        int      dst;  // the destination MPI rank of the message

        void invalidate() {
            src = INVALID;
        }

        bool isValid() const {
            return src != INVALID;
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

        MPITag_t generateMPITag_(); // generate a unique MPI tag
         // All messages sent by the current MPI process will have a unique tag.
         // Different processes will use the same tag but the combination of source MPI rank and tag is unique
         // The tag is written in the header of the Message, so that the receiver of the message knows it too.

    public: // data
        using Key_t = MessageHandlerKey_t;

        static std::vector<MessageHeaderContainer> theHeaders;
         // One MessageHeaderContainer per MPI rank

        static void broadcastMessageHeaders();

    public:
        MessageHeader     // Create a MessageHeader for receiving a message
          ( int src       // MPI source rank
          , int dst       // MPI destination rank
          , Key_t key     // MessageHandler key
          , size_t sz = 0 // size of message in bytes, usually set later (must be computed first
          );

        MessageHeader     // Create a MessageHeader for receiving a message
          ( int src       // MPI source rank from whom to receive
          , size_t i      // location in MessageHeaderContainer for MPI rank src
          );

     // Default copy ctor should be good to go

     // data member access:
        int       src()  const { return theHeaders[src_][i_].src; }
        int&      src()        { return theHeaders[src_][i_].src; }
        int       dst()  const { return theHeaders[src_][i_].dst; }
        int&      dst()        { return theHeaders[src_][i_].dst; }
        Key_t     key()  const { return theHeaders[src_][i_].key; }
        Key_t&    key()        { return theHeaders[src_][i_].key; }
        MPITag_t  tag()  const { return theHeaders[src_][i_].tag; }
        MPITag_t& tag()        { return theHeaders[src_][i_].tag; }
        size_t    size() const { return theHeaders[src_][i_].size; }
        size_t&   size()       { return theHeaders[src_][i_].size; }

        INFO_DECL;
        STATIC_INFO_DECL;

    private:
        void alloc_();
    };
    
 //------------------------------------------------------------------------------------------------
}// namespace mpi

#endif // MESSAGEHEADER_H
