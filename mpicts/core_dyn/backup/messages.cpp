#include "messages.h"

namespace mpi
{
 //-------------------------------------------------------------------------------------------------
 // Implementation of class MessageBody
 //-------------------------------------------------------------------------------------------------
    MessageBody::
    ~MessageBody()
    {
        if constexpr(::mpi::_debug_ && _debug_)
            prdbg("~MessageBody() : deleting MessageItems");
        for( auto p : msg_items_) {
            delete p;
        }
        if constexpr(::mpi::_debug_ && _debug_)
            prdbg("~MessageBody() : MessageItems deleted.");
    }



    void
    MessageBody::
    write
      ( void*& ptr // pointer where the message should be written to.
      ) const
    {
        MessageItemBase * const * pBegin = &msg_items_[0];
        MessageItemBase * const * pEnd   = pBegin + msg_items_.size();
        for( MessageItemBase * const * p = pBegin; p < pEnd; ++p) {
            (*p)->write(ptr);
        }
    }

    void
    MessageBody::
    read
      ( void*& ptr // pointer where the message should be read from.
      )
    {
        MessageItemBase ** pBegin = &msg_items_[0];
        MessageItemBase ** pEnd   = pBegin + msg_items_.size();
        for( MessageItemBase ** p = pBegin; p < pEnd; ++p) {
            (*p)->read(ptr);
        }
    }

    size_t // the number of bytes the mesage occupies in the MessageBuffer
    MessageBody::
    messageSize() const
    {
        size_t sz = 0;
        MessageItemBase * const * pBegin = &msg_items_[0];
        MessageItemBase * const * pEnd   = pBegin + msg_items_.size();
        for( MessageItemBase * const * p = pBegin; p < pEnd; ++p) {
            sz += (*p)->messageSize();
        }
        return sz;
    }

    void
    MessageBody::
    checkMessageBuffer()
    {
        size_t nBytes = messageSize();
        msg_buffer_.allocate(nBytes);
    }

//    Lines_t
//    MessageBody::
//    debug_text() const
//    {
//        Lines_t lines;
//        lines.push_back("message :");
//        size_t i = 0;
//        MessageItemBase * const * pBegin = &msg_items_[0];
//        MessageItemBase * const * pEnd   = pBegin + msg_items_.size();
//        for( MessageItemBase * const * p = pBegin; p < pEnd; ++p) {
//            lines.push_back( tostr("MessagaItem ",i++) );
//            Lines_t lines_i = (*p)->debug_text();
//            lines.insert(lines.end(), lines_i.begin(), lines_i.end());
//        }
//        return lines;
//    }



    void
    MessageBody::
    sendMessage()
    {
     // check buffer size
        checkMessageBuffer();
     // write buffer
        void* ptr = data();
        write( ptr );
     // send buffer
        MPI_Request request;
        int success =
        MPI_Isend                                       // non-blocking
          ( data()                          // pointer to buffer to send
          , messageEnd(msg_id) - messageBegin(msg_id)   // number of Index_t elements to send
          , MPI_LONG_LONG_INT                           // MPI type equivalent of Index_t
          , messageDestination(msg_id)                  // the destination
          , messageHandlerKey(msg_id)                   // the tag
          , MPI_COMM_WORLD
          , &request
          );
    }

 // Operations for receiving ////////////////////////////////////////////////////////////////////////
    void
    MessageBody::
    checkMessageBuffer // for receiving
      ( size_t messageSize // typically obtained from the corresponding MessageHeader.
      )
    {
        msg_buffer_.allocate(messageSize);
    }

    void
    MessageBody::
    receiveMessage() const; // const?

 //-------------------------------------------------------------------------------------------------
 // Implementation of class MessageCollection
 //-------------------------------------------------------------------------------------------------
    MessageBody&
    MessageCollection::
    addMessage                      // add empty message to this messageCollection
      ( int to_rank                 // the destination of the message
      , MessageHandler const& messageHandler   // the messageHandler
      )

 //-------------------------------------------------------------------------------------------------
 // Implementation of class Messages
 //-------------------------------------------------------------------------------------------------
    void
    Messages::
    communicate()
    {
     // Broadcast the number of message to all MPI ranks
        for( int source = 0; source < mpi::size; ++source ) {
            MPI_Bcast(&nMessages_[source], 1, MPI_LONG_LONG_INT, source, MPI_COMM_WORLD);
        }

        if constexpr(::mpi::_debug_)
        {// print the number of messages per rank:
            Lines_t lines;
            std::stringstream ss;
            for( int r = 0; r < mpi::size; ++r ) {
                ss<<"r"<<i<<std::setw(6)<<nMessages_[i];
                lines.push_back(ss.str()); ss.str(std::string());
            }
            prdbg( tostr("broadcast(): number of messages in each rank:"), lines );
        }

     // Adjust the size of each rank's MessageCollection:
        for( int r = 0; r < mpi::size; ++r ) {
            auto n = nMessages_[i];
            messageCollection(r).resize(n);
        }

     // Broadcast the header section of all MPI ranks:
     // All the headers to appear after each other, therefore the buffer location depends on the proces
     // Also note that we do NOT want to send the first entry of the buffer as this contains the number
     // of messages in the header.
        for( int source = 0; source < mpi::size; ++source ) {
            MPI_Bcast
            ( messageCollection(source).ptrToMessageHeaders()
            , messageCollection(source).sizeofMessageHeaders()
            , MPI_LONG_LONG_INT // MPI equivalent of Index_t
            , source
            , MPI_COMM_WORLD
            );
        }

        if constexpr(::mpi::_debug_ && _debug_)
        {// print the headers:
            prdbg("MessageBuffer::broadcast() : headers transferred:", headersToStr());
        }

     // Before the messages can be received, we must adjust each message's buffer size.
//        for( int r = 0; r < mpi::size; ++r )
//        {
//            if (r != rank )
//            {
//                MessageCollection messageCollection = messageCollection(r);
//                for( size_t iMsg=0; iMsg < messageCollection.size(); ++iMsg )
//                {
//                    messageSize = messageCollection.header(iMsg).size;
//                    messageCollection.body(iMsg).checkMessageBuffer(messageSize);
//                }
//            }
//        }

     // Send the message contents.

// Loop over the message headers in this messageBuffer.
//   If its source is this rank, then send it to its destination: MPI_Isend (non-blocking)
//   If its destination is this rank then
//     adjust the MessageBuffer
//     receive the message from this rank: MPI_Recv (blocking)
//   update the header if needed to have the correct locations of the begin and end of the message.
//
// What do we use as a tag? We need it as there may be several messages between the same rank.
// Since no process can make two message with the same messageHandlerKey, the latter can be used as a tag.
// The triplet (source rank, destination rank, messageHandlerKey) is unique.
//
// We first want do all the sends, non-blocking, then all the receives, blocking.
// This is automatically satisfied because all the sends are at the beginning of the
// messageBuffer.

     // First we do the sends. They are in the MessageCollection of the current MPI rank.
        MessageCollection& sends = messageCollection(mpi::rank);
        size_t nMessages = sends.size()
        for (size_t m=0; m < nMessages; ++m )
        {
            sends.body(i).sendMessage();
        }

     // Before doing the receives, we must adjust the MessageBuffers. We first adjust
     // all the buffers and then do all the receives because the receives are blocking.
        for( int r = 0; r < mpi::size; ++r )
        {
            if( r != mpi::rank )
            {
                MessageCollection& recvs = messageCollection(r);
                size_t nMessages = recvs.size()
                for (size_t m=0; m < nMessages; ++m )
                    recvs.checkReceiveMessageBuffer(i);
            }
        }
     // Now do the receives:
        for( int r = 0; r < mpi::size; ++r )
        {
            if( r != mpi::rank )
            {
                MessageCollection& recvs = messageCollection(r);
                size_t nMessages = recvs.size()
                for (size_t m=0; m < nMessages; ++m )
                    recvs.body(i).receiveMessage();
            }
        }
//        if constexpr(::mpi::_debug_ && _debug_)
//        {//
//            prdbg( tostr("MessageBuffer::broadcast() : sending   message content "
//                        , messageSource(msg_id), "->", messageDestination(msg_id)
//                        , ", key=", messageHandlerKey(msg_id))
//                 );
//        }


        int elements_added = 0;
        for( Index_t msg_id = 0; msg_id < nMessages(); ++msg_id)
        {
            if( messageSource(msg_id) == rank )
            {// send the message content
                if constexpr(::mpi::_debug_ && _debug_) {
                    prdbg( tostr("MessageBuffer::broadcast() : sending   message content "
                                , messageSource(msg_id), "->", messageDestination(msg_id)
                                , ", key=", messageHandlerKey(msg_id))
                         );
                }
                MPI_Request request;
                int success =
                MPI_Isend                                       // non-blocking
                  ( messagePtr(msg_id)                          // pointer to buffer to send
                  , messageEnd(msg_id) - messageBegin(msg_id)   // number of Index_t elements to send
                  , MPI_LONG_LONG_INT                           // MPI type equivalent of Index_t
                  , messageDestination(msg_id)                  // the destination
                  , messageHandlerKey(msg_id)                   // the tag
                  , MPI_COMM_WORLD
                  , &request
                  );
            } else {
                if( messageDestination(msg_id) == rank )
                {// recv the message content
                    if constexpr(::mpi::_debug_ && _debug_) {
                        prdbg( tostr("MessageBuffer::broadcast() : receiving message content "
                                    , messageSource(msg_id), "->", messageDestination(msg_id)
                                    , ", key=", messageHandlerKey(msg_id))
                             );
                    }

                    int elements_to_add =  messageEnd(msg_id) - messageBegin(msg_id); // this number does not change
                    Index_t begin = messageEnd(msg_id - 1); // first element of the buffer where the message content will be written
                     // even if this is the first message to be received, the end of the previous message in the buffer is
                     // correctly set, since that is a message that was constructed on this rank and therefore the end
                     // of the previous message exists and is correct.
                    Index_t end   = begin + elements_to_add; // past-the-end element of the buffer where the message content will be written

                    int succes =
                    MPI_Recv
                      ( &pBuffer_[begin]            // pointer to buffer where to store the message
                      , elements_to_add             // number of elements to receive
                      , MPI_LONG_LONG_INT
                      , messageSource(msg_id)       // source rank
                      , messageHandlerKey(msg_id)   // tag
                      , MPI_COMM_WORLD
                      , MPI_STATUS_IGNORE
                      );
                 // Update the header of the message, so that the message content can be read afterwards.
                    setMessageBegin(msg_id, begin);
                    setMessageEnd  (msg_id, end);

                 // print the messageBuffer:
                    if constexpr(::mpi::_debug_ && _debug_) {
                        prdbg( tostr( "MessageBuffer::broadcast() : received message content (msg_id=", msg_id, ", "
                                    , messageSource(msg_id), "->", messageDestination(msg_id), ")"
                                    )
                             , messageToStr(msg_id)
                             );
                    }
                }
                else {
                    if constexpr(::mpi::_debug_ && _debug_) {
                        prdbg( tostr("MessageBuffer::broadcast() : skipping message  "
                                    , messageSource(msg_id), "->", messageDestination(msg_id)
                                    , " because it it not for this rank (", rank, ")"
                                    )
                             );
                    }
                }
            }
        }
    }

 //-------------------------------------------------------------------------------------------------
    Messages theMessages;
 //------------------------------------------------------------------------------------------------
}// namespace mpi