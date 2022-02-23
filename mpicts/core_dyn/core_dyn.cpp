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
        prdbg("-*# test_MessageHandler() #*-");
        {
            double a = 5;                       //              8 bytes
            std::vector<int> ints = {1,2,3,4};  // 4*4 bytes = 16 bytes
                                                // size_t       8 bytes
                                                // --------------------
                                                //             32 bytes
            MessageHandler hndlr;
            prdbg(MessageHandler::static_info());
            hndlr.messageItemList().push_back(a);
            hndlr.messageItemList().push_back(ints);
            prdbg(MessageHandler::static_info());
            hndlr.addSendMessage(0);
            hndlr.addSendMessage(0);
            prdbg(MessageHeader::static_info());
            prdbg(hndlr.info("\n", "*0"));

            prdbg("MessageHeader::broadcastMessageHeaders()");
            MessageHeader::broadcastMessageHeaders();


            prdbg("hndlr.sendMessages()");
            hndlr.sendMessages();

            prdbg(hndlr.info("\n", "*1"));
            prdbg(MessageHandler::static_info());
            prdbg(MessageHeader::static_info());
        }
        finalize();
        return true;
    }
 //---------------------------------------------------------------------------------------------------------------------
}

PYBIND11_MODULE(core_dyn, m)
{// optional module doc-string
    m.doc() = "pybind11 core_dyn plugin"; // optional module docstring
 // list the functions you want to expose:
 // m.def("exposed_name", function_pointer, "doc-string for the exposed function");
    m.def("test_MessageHeader"    , &test::test_MessageHeader, "");
    m.def("test_MessageHandler"   , &test::test_MessageHandler, "");
}
