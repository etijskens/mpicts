#include "ParticleContainer.h"

namespace mpacts
{//---------------------------------------------------------------------------------------------------------------------
    ParticleContainer::
    ParticleContainer(int size)
      : r("r", *this)
      , m("m", *this)
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
    void
    ParticleContainer::
    prdbg()
    {
        std::stringstream ss;
        mpi::Lines_t lines;

        lines.push_back( ::tostr( "a", std::setw(4), "i", std::setw(4), "r", std::setw(4), "m"
//            <<std::setw(4)<<"x0"<<std::setw(4)<<"x1"<<std::setw(4)<<"x2"
                                )
                       );

        int size = alive_.size();
        for( int i=0; i<size; ++i)
        {
            lines.push_back( ::tostr( alive_[i], std::setw(4), i, std::setw(4), r[i], std::setw(4), m[i]
    //            <<std::setw(4)<<"x0"<<std::setw(4)<<"x1"<<std::setw(4)<<"x2"
                                    )
                           );
        }
        ::prdbg( tostr("ParticleContainer(size=", size, ")"), lines );
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
            ::prdbg(tostr("ParticleContainer.add() -> ", iFree));
        return iFree;
    }
 //---------------------------------------------------------------------------------------------------------------------
}// namespace mpacts