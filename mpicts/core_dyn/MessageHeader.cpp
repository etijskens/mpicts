#include "MessageHeader.h"

namespace mpi
{//------------------------------------------------------------------------------------------------
 // implementation of class MessageHeaderData
 //------------------------------------------------------------------------------------------------
    INFO_DEF(MessageHeaderData)
    {
        std::stringstream ss;
        ss<<indent<<"MessageHeaderData.info("<<title<<") : ( source="     <<source
                                                        <<", destination="<<destination
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
        ss<<indent<<"MessageHeaderContainer::info("<<title<<") : "
          <<indent<<"  ( nBytesPerHeader="<<nBytesPerHeader
                  <<  ", sizeof(MessageHeaderData)="<<sizeof(MessageHeaderData)
                  <<" ) ";
        if(headers_.size()) {
            for( size_t i=0; i < headers_.size(); ++i ){
                ss<<headers_[i].info( indent + "  ", std::string("i=") + std::to_string(i) );
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
      ( int src                 // MPI source rank
      , int dst                 // MPI source rank
      , MessageHandlerKey_t key //
      , size_t sz               // size of message in bytes, usually set later (must be computed first
      )
      : src_(src)
    {
        alloc_();
        MessageHeaderData& data = theHeaders[src][i_];
        data.key         = key;
        data.size        = sz;
        data.source      = src;
        data.destination = dst;
    }
 // todo:
 // Create a MessageHeader for receiving a message
    MessageHeader::
    MessageHeader
      ( MessageHeaderData* pMessageHeaderData
      )
      : src_(pMessageHeaderData->source)
    {// get the MessageHeaderContainer
     // get the index from the pointer
//        std::ptrdiff_t index = std::distance(objects.data(), p);
    }

    void
    MessageHeader::
    alloc_()
    {
        if( theHeaders.size() == 0 )
        {
            size_t sz = ( mpi::rank == -1 ) ? 1 : (size_t) mpi::size; // if mpi::init was not called, may occur during testing.
            theHeaders.resize( sz );
        }
        i_ = theHeaders[src_].addHeader();
    }

    STATIC_INFO_DEF(MessageHeader)
    {
        std::stringstream ss;
        ss<<indent<<"MessageHeader::static_info("<<title<<") : ";
        if( theHeaders.size() ) {
            ss<<"( size="<<theHeaders.size()<<" )";
            for( int rnk = 0; rnk < theHeaders.size(); ++rnk ) {
                ss<<theHeaders[rnk].info( indent + "  ", std::string("rank=") + std::to_string(rnk) );
            }
        } else {
            ss<<"( empty )";
        }
        return ss.str();
    }

    INFO_DEF(MessageHeader)
    {
        std::stringstream title_;
        title_<<"rank="<<src_<<", indx="<<i_;

        std::stringstream ss;
        ss<<theHeaders[src_][i_].info( indent + "  ", title_.str() );

        return ss.str();
    }

 //------------------------------------------------------------------------------------------------
}// namespace mpi
