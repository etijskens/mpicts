/*
 *  C++ source file for module mpicts.core_dyn
 */


// See http://people.duke.edu/~ccc14/cspy/18G_C++_Python_pybind11.html for examples on how to use pybind11.
// The example below is modified after http://people.duke.edu/~ccc14/cspy/18G_C++_Python_pybind11.html#More-on-working-with-numpy-arrays
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

namespace py = pybind11;

#include "mpicts.cpp"
#include "ParticleContainer.cpp"
using namespace mpacts;

#include "Messages.cpp"


namespace test7
{// Non-contiguous approach
 //---------------------------------------------------------------------------------------------------------------------
    bool test()
    {
        bool ok = false;
        init();

        std::cout<<::mpi::info<<" done"<<std::endl;
        finalize();
        return ok;
    }
}

PYBIND11_MODULE(core_dyn, m)
{// optional module doc-string
    m.doc() = "pybind11 core_dyn plugin"; // optional module docstring
 // list the functions you want to expose:
 // m.def("exposed_name", function_pointer, "doc-string for the exposed function");
    m.def("test7", &test7::test, "");
}
