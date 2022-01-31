#ifndef PARTICLECONTAINER_H
#define PARTICLECONTAINER_H
#include <Eigen/Geometry> //?

#include "mpicts.h"
using namespace mpi;

namespace mpacts
{//---------------------------------------------------------------------------------------------------------------------
    typedef Eigen::Matrix<float, 3, 1, Eigen::DontAlign> vec_t;
    typedef float real_t;

    class ParticleContainer;
 //---------------------------------------------------------------------------------------------------------------------
    template<typename T>
    class ParticleArray : public std::vector<T>
 //---------------------------------------------------------------------------------------------------------------------
    {
    private: // data members
        std::string name_;
        ParticleContainer& pc_;
    public:
        ParticleArray(std::string const& name, ParticleContainer& pc)
          : name_(name), pc_(pc)
        {}
        std::string const& name() const { return name_; }
        std::string const& particleContainer() const { return pc_; }
    };

 //---------------------------------------------------------------------------------------------------------------------
    class ParticleContainer
 //---------------------------------------------------------------------------------------------------------------------
    {
        static bool const _debug_ = false;
        std::vector<bool> alive_;
    public:
        ParticleArray<real_t> r;
        ParticleArray<real_t> m;
//        std::vector<vec_t>   x;

        ParticleContainer(int size);

        void prdbg();

        inline
        size_t // the size of a particle container.
        size() const {
            return alive_.size();
        }

     // Grow the arrays by a factor 1.5, and return the position of the first new element
     // (which is not alive, obviously).
        int grow();

     // Find an index for a new particle
        mpi::Index_t add();

     // remove an element
        inline void remove(int i) {
            alive_[i] = false;
        }

     // test if alive
        inline bool is_alive(int i) const {
            return alive_[i];
        }
    };
}// namespace mpacts

#endif // PARTICLECONTAINER_H