/*
 *  C++ source file for module mpicts.core_dyn
 */


// See http://people.duke.edu/~ccc14/cspy/18G_C++_Python_pybind11.html for examples on how to use pybind11.
// The example below is modified after http://people.duke.edu/~ccc14/cspy/18G_C++_Python_pybind11.html#More-on-working-with-numpy-arrays
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

namespace py = pybind11;

//#include "ParticleContainer.cpp"
//using namespace mpacts;

#include "mpicts.cpp"
#include "MessageData.cpp"
#include "MessageBuffer.cpp"
#include "MessageItemList.cpp"
#include "MessageHeader.cpp"
#include "MessageHandler.cpp"
#define PC
#ifdef PC
#  include "ParticleContainer.cpp"
#endif

using namespace mpi;

namespace test
{//---------------------------------------------------------------------------------------------------------------------
    bool test_MessageHeader()
    {
        std::cout<<"-*# test_MessageHeader() #*-"<<std::endl;

        MessageHeader mh0(0,2,3,4);
        std::cout<<mh0.info()<<std::endl;
        std::cout<<MessageHeader::static_info("1")<<std::endl;
        MessageHeader mh1(0,1,2,3);
        std::cout<<MessageHeader::static_info("2")<<std::endl;

        return true;
    }

 //---------------------------------------------------------------------------------------------------------------------
    bool test_MessageHandler()
    {
        init();
        assert( mpi::size >= 3
             && "At least 3 MPI processes are needed for this test."
              );
        prdbg("-*# test_MessageHandler() #*-");
        {
            double a = 5;                       //              8 bytes
            std::vector<int> ints = {1,2,3,4};  // 4*4 bytes = 16 bytes
                                                // size_t       8 bytes
                                                // --------------------
                                                //             32 bytes
            if( mpi::rank != 0 ) {
                a = 0;
                for( auto & i : ints ) i = 0;
            }
            MessageHandler hndlr;
            hndlr.messageItemList().push_back(a);
            hndlr.messageItemList().push_back(ints);
            prdbg(MessageHandler::static_info());
            prdbg(MessageHeader ::static_info("\n", "huh?"));

            if( mpi::rank == 0 )
            {// rank 0 sends the same message to ranks 1 and 2
                hndlr.addSendMessage(1);
                hndlr.addSendMessage(2);

                prdbg(MessageHeader::static_info());
                prdbg(hndlr.info("\n", "*0"));
            }

            prdbg("before MessageHeader::broadcastMessageHeaders()");
            MessageHeader::broadcastMessageHeaders();
            prdbg(tostr("after MessageHeader::broadcastMessageHeaders()"
                       , MessageHeader::static_info()
                       )
                 );

            prdbg("hndlr.sendMessages()");
            hndlr.sendMessages();
            hndlr.recvMessages();

            prdbg(hndlr.info("\n", "*1"));
            prdbg(MessageHandler::static_info());
            prdbg(MessageHeader::static_info());
            prdbg( tostr( "\na="      , a
                        , "\nints[0]=", ints[0]
                        , "\nints[1]=", ints[1]
                        , "\nints[2]=", ints[2]
                        , "\nints[3]=", ints[3]
                        )
                 );
        }
        finalize();
        return true;
    }

 //---------------------------------------------------------------------------------------------------------------------
#ifdef PC
    bool test_PcMessageHandler()
    {// Move the odd particles to the next rank

        init();
        assert( mpi::size >= 2
             && "At least 2 MPI processes are needed for this test."
              );
        prdbg("-*# test_PcMessageHandler() #*-");

        ParticleContainer pc(8);
        pc.prdbg();

        PcMessageHandler pcmh(pc);

        int dst = mpi::next_rank();
        Indices_t indices = {1,3,5,7};
        bool move = true;
        pcmh.addSendMessage(dst, indices, move);
        {
        }
        finalize();
        return true;
    }
#endif
 //---------------------------------------------------------------------------------------------------------------------
}

PYBIND11_MODULE(core_dyn, m)
{// optional module doc-string
    m.doc() = "pybind11 core_dyn plugin"; // optional module docstring
 // list the functions you want to expose:
 // m.def("exposed_name", function_pointer, "doc-string for the exposed function");
    m.def("test_MessageHeader"    , &test::test_MessageHeader, "");
    m.def("test_MessageHandler"   , &test::test_MessageHandler, "");
#ifdef PC
    m.def("test_PcMessageHandler" , &test::test_PcMessageHandler, "");
#endif
}
