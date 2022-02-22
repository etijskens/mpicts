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
    bool test_MessageBuffer()
    {
        std::cout<<"-*# test_MessageBuffer() #*-"<<std::endl;

        Buffer b(10);
        std::cout<<b.info("b")<<std::endl;
        {
            MessageBuffer sb(&b);
            std::cout<<sb.info("sb")<<std::endl;
            MessageBuffer sb2;
            sb2 = sb;
            std::cout<<sb.info("sb")<<std::endl;
            std::cout<<sb2.info("sb2")<<std::endl;
        }
        std::cout<<b.info("b final")<<std::endl;
        return true;
    }

 //---------------------------------------------------------------------------------------------------------------------
    bool test_MessageBufferPool()
    {
        std::cout<<"-*# test_MessageBufferPool() #*-"<<std::endl;

        {
            MessageBuffer sb0 = Buffer::getBuffer(10);
            std::cout<<sb0.info()<<std::endl;
            std::cout<<Buffer::static_info()<<std::endl;
            MessageBuffer sb1 = Buffer::getBuffer(20);
            std::cout<<sb1.info()<<std::endl;
            std::cout<<Buffer::static_info()<<std::endl;
        }
        std::cout<<Buffer::static_info()<<std::endl;
        return true;
    }

 //---------------------------------------------------------------------------------------------------------------------
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
//            prdbg(tostr("{a,ints} messageBufferSize = ", hndlr.computeMessageBufferSize()));
            hndlr.addMessage(0);
            hndlr.addMessage(0);
            prdbg(MessageHeader::static_info());
            prdbg(hndlr.info());
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
    m.def("test_MessageBuffer"    , &test::test_MessageBuffer, "");
    m.def("test_MessageBufferPool", &test::test_MessageBufferPool, "");
    m.def("test_MessageHeader"    , &test::test_MessageHeader, "");
    m.def("test_MessageHandler"   , &test::test_MessageHandler, "");
}
