#ifndef TRANSMITTER_H
#define TRANSMITTER_H

#include <vector>
#include "mpicts.h"
#include "MessageSet.h"

namespace mpi
{//------------------------------------------------------------------------------------------------
    class Transmitter
    {
        std::vector<MessageSet> processMessages_; // One MessageSet per MPI proces
        std::vector<Index_t> nMessages_; // number of message in each MPI rank
    public:
        Transmitter();

        void transmit();
         // Transmit all messages. That means:
         //   - send all messages of the current MPI rank to their destinations, and
         //   - receive the messages from other MPI rank meant for the current rank.
    private:
     // transmit() components:
        void encodeMessages_();
        void sendHeaders_();
        void sendMessages_();
        void receiveMessages_();
        void decodeMessages_();
    };
 //------------------------------------------------------------------------------------------------
}// namespace mpi

#endif // TRANSMITTER_H
