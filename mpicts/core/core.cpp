/*
 *  C++ source file for module mpicts.core
 */

#include <pybind11/pybind11.h>

namespace py = pybind11;
#include "mpicts.cpp"
#include "ParticleContainer.cpp"
using namespace mpacts;

#include "MessageBuffer.cpp"
#include "Message.cpp"
#include "MessageHandler.cpp"

#include <stdexcept>


using namespace mpi;

namespace test7
{// Non-contiguous approach
 //---------------------------------------------------------------------------------------------------------------------
    class PcMessageHandler : public MessageHandlerBase
    {
    private: // data members
        ParticleContainer& pc_;
        MessageItem<ParticleContainer>* ptr_pc_message_item_;
        Index_t nParticles_;
    public:
     // ctor
        PcMessageHandler(ParticleContainer& pc)
          : pc_(pc)
        {
         // Add the particle container subset
         // The corresponding MessageItem will only write the number of indices.
            ptr_pc_message_item_ = message().push_back(pc_);
         // Add the arrays
         // todo: use list of names as argument
            message().push_back(pc_.r, ptr_pc_message_item_);
            message().push_back(pc_.m, ptr_pc_message_item_);
        }

     // Select the particles to be moved/copied
        void
        select
          ( Indices_t& indices // list of particle indices to be moved/copied
          , bool move = true   // Delete the particles after the message is written. This means that the selected
                               // particles are moved to another domain. Otherwise, they are copied.
          )
        {
            ptr_pc_message_item_->select(indices, move);
        }

     // Write the message to theMessageBuffer.
        virtual
        Index_t // message id of the message in the buffer. Rarely used.
        writeMessage
          ( int to_rank        // destination proces MPI rank
          , Indices_t& indices // list of indices to be copied/moved
          , bool move = true   // move or copy particles to proces for_rank
          )
        {
            select(indices, move);
            return MessageHandlerBase::writeMessage(to_rank);
        }
    };

    bool test()
    {
        init();

        ParticleContainer pc(8);
        pc.prdbg();

        bool ok = true;
        PcMessageHandler pcmh(pc);
     // move the odd particles to the next rank
     // there is one message for each process
        Indices_t indices = {1,3,5,7};

        pcmh.writeMessage( next_rank(), indices );

        mpi::theMessageBuffer.broadcast();
        mpi::theMessageBuffer.readMessages();
        pc.prdbg();

        Lines_t lines;
        for(size_t i=0; i<pc.size(); ++i )
        {
            if( pc.is_alive(i) )
            {
                std::stringstream ss;
                ss<<std::setw(4)<<i
                  <<std::setw(4)<<pc.r[i]
                  <<std::setw(4)<<pc.m[i]
//                  <<" ["<<std::setw(4)<<pc.x[i][0]<<std::setw(4)<<pc.x[i][1]<<std::setw(4)<<pc.x[i][2]<<"]"
                  <<" : ";
             // verify contents:
                int const   my_rank = rank;
                int const prev_rank = next_rank(-1);
                real_t expected_r = ( i%2 ? 100*prev_rank + i // odd i
                                           : 100*my_rank + i   //even i
                                     );
                real_t expected_m = expected_r + 8;
    //            vec_t   expected_x = vec_t(expected_r, expected_r+8, expected_r+16);

                ok &= pc.r[i] == expected_r;
                ss<<' '<<ok;

                ok &= pc.m[i] == expected_m;
                ss<<','<<ok;
    //            ok &= pc.x[i] == expected_x;
    //            ss <<' '<<ok<<;

                lines.push_back(ss.str());
            }
        }
        prdbg("pc verify", lines);

        std::cout<<::mpi::info<<" done"<<std::endl;
        finalize();
        return ok;
    }
 //---------------------------------------------------------------------------------------------------------------------
}// namespace test7

PYBIND11_MODULE(core, m)
{// optional module doc-string
    m.doc() = "pybind11 core plugin"; // optional module docstring
 // list the functions you want to expose:
 // m.def("exposed_name", function_pointer, "doc-string for the exposed function");
//    m.def("test6", &test6::test, "");
    m.def("test7", &test7::test, "");
}
