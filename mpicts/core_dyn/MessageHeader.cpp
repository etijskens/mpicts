#include "MessageHeader.h"

namespace mpi
{//------------------------------------------------------------------------------------------------
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

    std::string
    MessageHeaderContainer::
    info( std::string const& s ) const
    {
        std::stringstream ss;
        ss  <<  "MessageHeaderContainer::info("<<s<<")"
            <<"\n    nBytesPerHeader           : "<<nBytesPerHeader
            <<"\n    sizeof(MessageHeaderData) : "<<sizeof(MessageHeaderData)
            <<"\n    size                      : "<<headers_.size()
            ;
        for( size_t i=0; i < headers_.size(); ++i ){
            MessageHeaderData const h = headers_[i];
            ss<<"\n    "<<i<<" { src="<<h.source<<", dst="<<h.destination<<", key="<<h.key<<", size="<<h.size<<" }";
        }
        ss<<std::endl;
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

    std::string
    MessageHeader::
    theHeadersInfo( std::string const& s )
    {
        std::stringstream ss;
        ss  <<"All MessageHeaderContainers: size="<<theHeaders.size() ;
        for( int rnk = 0; rnk < theHeaders.size(); ++rnk )
        {
            ss<<"\nMPI rank "<<rnk<<": "
              <<theHeaders[rnk].info();
        }
        ss<<std::endl;
        return ss.str();
    }

    std::string
    MessageHeader::
    info( std::string const& s ) const
    {
        std::stringstream ss;
        MessageHeaderData const h = theHeaders[src_][i_];
        ss
        <<  "MessageHeader: "<<s
        <<"\n    src :"<< src_
        <<"\n    loc :"<< i_
        <<"\n    { src="<<h.source<<", dst="<<h.destination<<", key="<<h.key<<", size="<<h.size<<" }"
        <<std::endl;
        return ss.str();
    }

 //------------------------------------------------------------------------------------------------
}// namespace mpi
