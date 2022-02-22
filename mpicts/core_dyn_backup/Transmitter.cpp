#include "Transmitter.h"

namespace mpi
{//-------------------------------------------------------------------------------------------------
 // Implementation of class Transmitter
 //-------------------------------------------------------------------------------------------------
    Transmitter::
    Transmitter()
    {
        if( mpi::isInitialized() && mpi::size > 1 )
        {// Construct a MessageSet for each rank
            processMessages_.resize(mpi::size);
        }
    }

    void
    Transmitter::
    addMessage(MessageHandlerBase& messageHandler, int destination, OptionalMessageData const* md)
    {
        MessageSet& myMessages = processMessages_[mpi::rank];

    }


    void
    Transmitter::
    transmit()
    {
        computeBufferSizes_();
        sendHeaders_();
        encodeMessages_();
        sendMessages_();
        receiveMessages_();
        decodeMessages_();
    }

    void
    Transmitter::
    computeBufferSizes_()
    {
        MessageSet& thisRanksMessages = processMessages_[mpi::rank];
        for ( MessageSet::iterator iter = thisRanksMessages.begin()
            ; iter != thisRanksMessages.end()
            ; ++iter )
        {
            iter->computeBufferSize();
        }
    }

    void
    Transmitter::
    sendHeaders_()
    {
     // first broadcast the number of messages to all MPI ranks
        for( int source = 0; source < mpi::size; ++source ) {
            MPI_Bcast(&nMessages_[source], 1, MPI_LONG_LONG_INT, source, MPI_COMM_WORLD);
        }

//        if constexpr(::mpi::_debug_)
//        {// print the number of messages per rank:
//            Lines_t lines;
//            std::stringstream ss;
//            for( int r = 0; r < mpi::size; ++r ) {
//                ss<<"r"<<i<<std::setw(6)<<nMessages_[i];
//                lines.push_back(ss.str()); ss.str(std::string());
//            }
//            prdbg( tostr("broadcast(): number of messages in each rank:"), lines );
//        }

     // Adjust the size of MessageSet of the receiving ranks:
        for( int rnk = 0; rnk < mpi::size; ++rnk ) {
            if( rnk != mpi::rank ) {
                auto n = nMessages_[rnk];
                processMessages_[rnk].resize(n);
            }
        }

     // Broadcast the header section of all MPI ranks:
     // All the headers to appear after each other, therefore the buffer location depends on the proces
     // Also note that we do NOT want to send the first entry of the buffer as this contains the number
     // of messages in the header.
        for( int source = 0; source < mpi::size; ++source ) {
            MPI_Bcast
            ( processMessages_[source].pHeaders()
            , processMessages_[source].size() * sizeof(MessageHeader) // # of bytes
            , MPI_CHAR
            , source
            , MPI_COMM_WORLD
            );
        }

//        if constexpr(::mpi::_debug_ && _debug_)
//        {// print the headers:
//            prdbg("MessageBuffer::broadcast() : headers transferred:", headersToStr());
//        }
    }

    void
    Transmitter::
    encodeMessages_()
    {
        MessageSet& thisRanksMessages = processMessages_[mpi::rank];
        for ( MessageSet::iterator iter = thisRanksMessages.begin()
            ; iter != thisRanksMessages.end()
            ; ++iter )
        {
            iter->adjustBuffer();
            iter->writeBuffer();
        }
    }

    void
    Transmitter::
    sendMessages_()
    {
        MessageSet& thisRanksMessages = processMessages_[mpi::rank];
        for ( MessageSet::iterator iter = thisRanksMessages.begin()
            ; iter != thisRanksMessages.end()
            ; ++iter )
        {
            iter->adjustBuffer();
            iter->writeBuffer();
            iter->sendBuffer();
        }
    }

    void
    Transmitter::
    receiveMessages_()
    {// First adjust the buffers for the messages to be received.
     // Loop over the MessageSets of all ranks
        for( int rnk = 0; rnk < mpi::size; ++rnk )
        {// skip the current rank's MessageSet as it is for sending, not receiving:
            if( rnk != mpi::rank )
            {
                MessageSet& messages = processMessages_[rnk];
                for ( MessageSet::iterator iter = messages.begin()
                    ; iter != messages.end()
                    ; ++iter )
                {// we must only provide buffers for messages sent to the current rank
                    if ( iter->destination() == mpi::rank )
                        iter->adjustBuffer();
                }
            }
        }
     // Ready for receiving: we make exactly the same loop as above
        for( int rnk = 0; rnk < mpi::size; ++rnk ) {
            if( rnk != mpi::rank ) {
                MessageSet& messages = processMessages_[rnk];
                for ( MessageSet::iterator iter = messages.begin()
                    ; iter != messages.end()
                    ; ++iter )
                {// we must only receive messages sent to the current rank
                    if ( iter->destination() == mpi::rank )
                        iter->recvBuffer();
                }
            }
        }
    }

    void
    Transmitter::
    decodeMessages_()
    {
     // Again we make exactly the same loop as above
        for( int rnk = 0; rnk < mpi::size; ++rnk ) {
            if( rnk != mpi::rank ) {
                MessageSet& messages = processMessages_[rnk];
                for ( MessageSet::iterator iter = messages.begin()
                    ; iter != messages.end()
                    ; ++iter )
                {// we must only receive messages sent to the current rank
                    if ( iter->destination() == mpi::rank )
                        iter->readBuffer();
                }
            }
        }
    }
 //-------------------------------------------------------------------------------------------------
    Transmitter theTransmitter;
 //-------------------------------------------------------------------------------------------------
}// namespace mpi