/*
 *  C++ source file for module mpicts.core_dyn
 */


// See http://people.duke.edu/~ccc14/cspy/18G_C++_Python_pybind11.html for examples on how to use pybind11.
// The example below is modified after http://people.duke.edu/~ccc14/cspy/18G_C++_Python_pybind11.html#More-on-working-with-numpy-arrays
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

namespace py = pybind11;

#include "ParticleContainer.cpp"
using namespace mpacts;

#include "mpicts.cpp"
#include "MessageItemList.cpp"
#include "MessageHandler.cpp"
#include "MessageBuffer.cpp"
#include "MessageSet.cpp"
#include "Transmitter.cpp"

namespace test7
{// Non-contiguous approach
 //---------------------------------------------------------------------------------------------------------------------
    class PcMessageHandler : public MessageHandlerBase
 // MessageHandler for a ParticleContainer
 // There must be one PcMessageHandler object for every communicating ParticleContainer.
 //---------------------------------------------------------------------------------------------------------------------
    {
    private: // data members
        ParticleContainer* ptr_pc_;
        MessageItem<ParticleContainer>* ptr_pcMessageItem_;
        Index_t nParticles_;
    public:
     // ctor
        PcMessageHandler()
          : ptr_pc_(nullptr), ptr_pcMessageItem_(nullptr)
        {}

        void init(ParticleContainer& pc)
        {
            ptr_pc_ = &pc;
         // Add the particle container subset
         // The corresponding MessageItem will only write the number of selected particles.
            ptr_pcMessageItem_ = messageItemList().push_back(pc);
         // Add the arrays
         // todo: use list of array names as argument and a suitable default.
            messageItemList().push_back(pc.r, ptr_pcMessageItem_);
            messageItemList().push_back(pc.m, ptr_pcMessageItem_);
        }

     // Select the particles to be moved/copied
        void
        addMessage
          ( int destination    // destination MPI rank
          , Indices_t& indices // list of particle indices to be moved/copied to this ParticleContainer in the destination MPI process
          , bool move = true   // Delete the particles after the message is written. This means that the selected
                               // particles are moved to another domain. Otherwise, they are copied.
          )
        {
            ptr_pcMessageItem_->select(indices, move);
        }

     // Write the message to theMessageBuffer.
        virtual
        Index_t // message id of the message in the buffer. Rarely used.
        writeBuffer
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
        PcMessageHandler& pcmh = theMessageHandlerRegistry.create<PcMessageHandler>();
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
}

PYBIND11_MODULE(core_dyn, m)
{// optional module doc-string
    m.doc() = "pybind11 core_dyn plugin"; // optional module docstring
 // list the functions you want to expose:
 // m.def("exposed_name", function_pointer, "doc-string for the exposed function");
    m.def("test7", &test7::test, "");
}
