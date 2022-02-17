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
#include "MessageBuffer.cpp"
#include "MessageItemList.cpp"
#include "MessageHeader.cpp"
#include "MessageHandler.cpp"

using namespace mpi;

namespace test
{//---------------------------------------------------------------------------------------------------------------------
    bool test_MessageBuffer()
    {
        Buffer b(10);
        std::cout<<b.info("b")<<std::endl;
        {
            SharedBuffer sb(&b);
            std::cout<<sb.info("sb")<<std::endl;
            SharedBuffer sb2;
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
        MessageBufferPool pool;
        pool.info();
        {
            SharedBuffer sb0 = pool.getBuffer(10);
            sb0.info();
            pool.info();
            SharedBuffer sb1 = pool.getBuffer(20);
            sb1.info();
            pool.info();
        }
        pool.info();
        return true;
    }

 //---------------------------------------------------------------------------------------------------------------------
    bool test_MessageHeader()
    {
        MessageHeader mh0(0,2,3,4);
        mh0.info();
        MessageHeader::theHeadersInfo();
//        MessageHeader::theHeaders[0].info("1");
//        MessageHeader mh1(0,1,2,3);
//        MessageHeader::theHeaders[0].info("1");

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
}
