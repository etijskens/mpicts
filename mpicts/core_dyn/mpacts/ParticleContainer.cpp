#include "ParticleContainer.h"

namespace mpacts
{//---------------------------------------------------------------------------------------------------------------------
    ParticleContainer::
    ParticleContainer(int size, std::string const& name)
      : r("r", *this)
      , m("m", *this)
      , name_(name)
    {
        alive_.resize(size);
        r.resize(size);
        m.resize(size);
//            x.resize(size);
        for( int i=0; i<size; ++i) {
            int ir = 100*mpi::rank + i;
            alive_[i] = true;
            r[i] = ir;
            m[i] = ir + size;
//                for( int k=0; k<3; ++k )
//                    x[i][k] = ir+k*size;
        }
    }

 //---------------------------------------------------------------------------------------------------------------------
    INFO_DEF(ParticleContainer)
    {
        std::stringstream ss;
        ss<<indent<<"ParticleContainer::info("<<title<<") : name="<<name()<<", size="<<alive_.size()
          <<indent<<"  a=alive[i], i=particle index, r=position[i], m=mass[i]"
          <<indent<<"  a"<<std::setw(4)<<"i"<<std::setw(4)<<"r"<<std::setw(4)<<"m";
        for( int i=0; i<alive_.size(); ++i) {
            ss<<indent<<"  "<<alive_[i]<<std::setw(4)<<i<<std::setw(4)<<r[i]<<std::setw(4)<<m[i];
        }
        return ss.str();
    }

 //---------------------------------------------------------------------------------------------------------------------
 // Grow the arrays by a factor 1.5, and return the position of the first new element
 // (which is not alive, obviously).
    int
    ParticleContainer::
    grow()
    {
        int old_size = alive_.size();
        int new_size = (int)(old_size*1.5);
        if (new_size == old_size) ++new_size;
        alive_.resize(new_size);
//            x.resize(new_size);
        r.resize(new_size);
        m.resize(new_size);
        for (int i=old_size; i<new_size; ++i) {
            alive_[i] = false;
        }
        return old_size;
    }

 //---------------------------------------------------------------------------------------------------------------------
 // Find or create an index for a new particle
    mpi::Index_t
    ParticleContainer::
    add()
    {// look for a dead particle
        mpi::Index_t iFree = -1;
        for( Index_t i=0; i<alive_.size(); ++i) {
            if (not alive_[i]) {
                iFree = i;
                break;
            }
        }
        if( iFree == -1 )
        {// none found, grow the array.
            iFree = grow();
        }
        alive_[iFree] = true;
        if constexpr(::_debug_ && _debug_)
            ::prdbg(concatenate("ParticleContainer.add() -> ", iFree));
        return iFree;
    }
}// namespace mpacts
