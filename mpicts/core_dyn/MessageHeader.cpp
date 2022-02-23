#include "MessageHeader.h"
#include "MessageHandler.h"

namespace mpi
{//------------------------------------------------------------------------------------------------
 // implementation of class MessageHeaderData
 //------------------------------------------------------------------------------------------------
    INFO_DEF(MessageHeaderData)
    {
        std::stringstream ss;
        ss<<indent<<"MessageHeaderData.info("<<title<<") : ( source="     <<src
                                                        <<", destination="<<dst
                                                        <<", key="        <<key
                                                        <<", size="       <<size
                                                        <<" )";
        return ss.str();
    };

 //------------------------------------------------------------------------------------------------
 // implementation of class MessageHeaderContainer
 //------------------------------------------------------------------------------------------------
    size_t MessageHeaderContainer::nBytesPerHeader = 0;

    size_t       // return the index of the added MessageHeaderData in headers_
    MessageHeaderContainer::
    addHeader() // add an empty message header
    {
        size_t i = headers_.size();
        headers_.resize( i + 1 );
        headers_.back().invalidate();
        return i;
    }

    MessageHeaderContainer::
    MessageHeaderContainer()
    {
        computeNBytesPerHeader_();
    }

    void
    MessageHeaderContainer::
    computeNBytesPerHeader_()
    {
        if( MessageHeaderContainer::nBytesPerHeader == 0 )
        {
            size_t sz = headers_.size();
            if( sz < 2 )
            {// we need at least two successive elements
                headers_.resize(2);
            }

            std::ptrdiff_t diff = (char*)(&headers_[1]) - (char*)(&headers_[0]);
            MessageHeaderContainer::nBytesPerHeader = diff;

         // reset original size.
            headers_.resize(sz);
        }
    }

    INFO_DEF(MessageHeaderContainer)
    {
        std::stringstream ss;
        ss<<indent<<"MessageHeaderContainer::info("<<title<<") :";
        if(headers_.size()) {
            for( size_t i=0; i < headers_.size(); ++i ){
                ss<<headers_[i].info( indent + "  ", tostr("header ", i, " of ", headers_.size()) );
            }
        } else {
            ss<<indent<<"  ( empty )";
        }
        return ss.str();
    }

 //------------------------------------------------------------------------------------------------
 // implementation of class MessageHeader
 //------------------------------------------------------------------------------------------------
    std::vector<MessageHeaderContainer> MessageHeader::theHeaders;

//    MessageHeader::
//    MessageHeader()
//    {
//        alloc_();
//    }

 // Create a MessageHeader for sending a message
    MessageHeader::
    MessageHeader
      ( int src   // MPI source rank
      , int dst   // MPI source rank
      , Key_t key // MessageHandler key
      , size_t sz // size of message in bytes, usually computed later
      )
      : src_(src)
    {
        alloc_();
        MessageHeaderData& data = theHeaders[src][i_];
        data.key  = key;
        data.size = sz;
        data.src  = src;
        data.dst  = dst;
    }

 // Create a MessageHeader for receiving a message
    MessageHeader::
    MessageHeader
      ( int src       // MPI source rank
      , size_t i      // location in MessageHeaderContainer
      )
      : src_(src)
      , i_(i)
    {}

 //------------------------------------------------------------------------------------------------
    void
    MessageHeader::
    alloc_()
    {
        if( theHeaders.size() == 0 )
        {// Make sure that there is a MessageHeaderContainer for every MPI rank, or, if mpi was not
         // initialized (which may occur during testing), that there is at least one MessageHeaderContainer.
            size_t sz = ( mpi::rank == -1 ) ? 1 // if mpi::init() was not called
                                            : (size_t) mpi::size;
            theHeaders.resize( sz );
        }
     // allocate a header in this rank's MessageHeaderContainer
        i_ = theHeaders[src_].addHeader();
         // if mpi::init() was not called src_ is 0
    }

 //------------------------------------------------------------------------------------------------
    STATIC_INFO_DEF(MessageHeader)
    {
        std::stringstream ss;
        ss<<indent<<"MessageHeader::static_info("<<title<<") :"
          <<indent<<"  ( nBytesPerHeader="<<MessageHeaderContainer::nBytesPerHeader
                  <<  ", sizeof(MessageHeaderData)="<<sizeof(MessageHeaderData)
                  <<" ) ";
        if( theHeaders.size() ) {
            for( int rnk = 0; rnk < theHeaders.size(); ++rnk ) {
                ss<<theHeaders[rnk].info( indent + "  ", tostr("rank ", rnk, " of ", theHeaders.size()) );
            }
        } else {
            ss<<"( empty )";
        }
        return ss.str();
    }

 //------------------------------------------------------------------------------------------------
    INFO_DEF(MessageHeader)
    {
        std::stringstream title_;
        title_<<"rank="<<src_<<", indx="<<i_;

        std::stringstream ss;
        ss<<theHeaders[src_][i_].info( indent, title_.str() );

        return ss.str();
    }

 //------------------------------------------------------------------------------------------------
 // static member function
    void
    MessageHeader::
    broadcastMessageHeaders()
    {
     // Before the MessageHeaders can be broadcasted, the buffer sizes must be computed and stored in the
     // MessageHeaders!
        MessageHeaderContainer& myHeaders = theHeaders[mpi::rank];
        for( size_t i = 0; i < myHeaders.size(); ++ i ) // loop over all MessageHeaders created by this rank
        {
            MessageHeaderData& messageHeaderData = myHeaders[i];
            MessageHandler& hndlr = MessageHandler::theMessageHandlerRegistry[messageHeaderData.key];
            hndlr.computeMessageBufferSizes();
        }

        if( mpi::size > 1)
        {// Make sure that every MPI rank knows how many messages the other ranks are sending
            std::vector<size_t> nMessagesInRank(mpi::size);
            nMessagesInRank[mpi::rank] = theHeaders[mpi::rank].size();
            for( int source = 0; source < mpi::size; ++source ) {
                MPI_Bcast(&nMessagesInRank[source], 1, MPI_SIZE_T, source, MPI_COMM_WORLD);
            }

         // Adjust the size of the header section for the other ranks according to nMessagesInRank,
         // to allow receiving their headers.
            for( size_t rnk = 0; rnk < mpi::size; ++rnk ) {
                if( rnk != mpi::rank ) {
                    theHeaders[rnk].resize( nMessagesInRank[rnk] );
                }
            }

         // Broadcast the header sections of all processes
            for( int source = 0; source < mpi::size; ++source ) {
                MPI_Bcast
                ( &(theHeaders[source][0])
                , theHeaders[source].size() * sizeof(MessageHeader) // # of bytes
                , MPI_CHAR
                , source
                , MPI_COMM_WORLD
                );
            }

         // loop over all messages and create MessageData for each message to receive
            for( int src = 0; src < mpi::size; ++src ) {// loop over all senders
                if( src != mpi::rank ) {// we are not sending to / receiving from ourselve
                    MessageHeaderContainer& srcHeaders = theHeaders[rank];
                    for( size_t i = 0; i < srcHeaders.size(); ++i ) {// loop over all message from src
                        if( srcHeaders[i].dst == mpi::rank ) {// this is a message for me
                            MessageHandler& hndlr = MessageHandler::theMessageHandlerRegistry[srcHeaders[i].key];
                            hndlr.addRecvMessage(src, i);
                        }
                    }
                }
            }
        }
    }
 //------------------------------------------------------------------------------------------------
}// namespace mpi
