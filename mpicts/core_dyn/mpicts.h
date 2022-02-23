#ifndef MPICTS_H
#define MPICTS_H

#include <mpi.h>

#include <iomanip>
#include <sstream>
#include <vector>
#include <string>
#include <Eigen/Geometry> //?


#define CINFO INFO.c_str()

/*
    https://stackoverflow.com/questions/35324597/writing-to-text-file-in-mpi
    https://www.geeksforgeeks.org/variadic-function-templates-c/
    https://en.cppreference.com/w/cpp/language/parameter_pack
    https://eli.thegreenplace.net/2014/variadic-templates-in-c/
 */

namespace mpi // this code is both for the one-sided approach and for the two-sided approach.
{//---------------------------------------------------------------------------------------------------------------------
 // A compile time constant restricted to this namespace. Set to true and recompile to produce debug output.
    bool const _debug_ = true;

 // Global variables set by mpi::init()
    extern int rank;
    extern int size;
    extern std::string INFO; // "MPI rank [rank/size]", useful for debugging messages

 //---------------------------------------------------------------------------------------------------------------------
 // typedefs
    using Lines_t = std::vector<std::string>;
    using MessageHandlerKey_t = size_t;
     // This type may be modified, but there are some constraints:
     // Because we need to know the key before the message can be received, it is necessary
     // that it can be stored in the header section of MessageBuffers. Therefor, it must be
     // of fixed size, and, preferentially, a size that is a multiple of sizeof(Index_t).

    using Index_t = int64_t; // copied from Primitives/Types/Index.h
    using Indices_t = std::vector<Index_t>; // list of indices

 //---------------------------------------------------------------------------------------------------------------------
 // Initialize MPI
    void init();

 //---------------------------------------------------------------------------------------------------------------------
 // Check if MPI is initialized
    bool isInitialized();

 //---------------------------------------------------------------------------------------------------------------------
 // finalize MPI
    void finalize();

 //---------------------------------------------------------------------------------------------------------------------
 // get the rank corresponding to rank_ + n
    inline int // result is always in [0,size_[ (as with periodic boundary conditions)
    next_rank
      ( int n = 1 // number of ranks to move, n>0 moves upward, n<0 moves down.
      )
    {
        int r = rank + n;
        while( r < 0 ) r += size;
        return r % size;
    }

 //---------------------------------------------------------------------------------------------------------------------
 // Machinery for producing debugging output
 //---------------------------------------------------------------------------------------------------------------------
    extern std::string dbg_fname;
    extern const Lines_t nolines;
    extern int64_t timestamp0;

 // Produce debug output (in file `_<rank>.dbg`). Writes a
 // - timestamp
 // - a <title> string
 // - optionally some <lines>
 // - a terminating line (dashes)
 // - an empty line
    void
    prdbg
      ( std::string const& title     // the title string
      , Lines_t const& lines=nolines // optional lines
      );

 // Write <last> to the stringstream <ss>
    template<typename Last>
    void
    tostrhelper
      ( std::stringstream & ss
      , Last const& last
      )
    {
        ss<<last;
    }

 // Write a variable number of arguments to the stringstream <ss>
    template<typename First, typename... Args>
    void
    tostrhelper
      ( std::stringstream & ss
      , First const& first     // first argument to write to <ss>
      , Args... args           // remaining arguments to write to <ss>
      )
    {
        ss<<first;
        tostrhelper(ss, args...);
    }

 // Produce a std::string from a series of arguments. Internally, the arguments are consecutively fed into a
 // std::stringstream (so you can e.g. use std::setw(4) as an argument to set the width for the next argument).
 // Often used to construct the first argumente of prdbg's first argument, and to construct the lines of prdbg's
 // second argument.
    template<typename... Args>
    std::string
    tostr(Args... args)
    {
        std::stringstream ss;
        tostrhelper(ss, args...);
        return ss.str();
    }

    template<typename T>
    Lines_t tolines( std::string const& s, std::vector<T> const & v)
    {
        Lines_t lines;
        std::stringstream ss;
        ss<<s<<"(size="<<v.size()<<") [";
        lines.push_back( ss.str()); ss.str(std::string());
        for( size_t i = 0; i < v.size(); ++i ) {
            ss<<std::setw(6)<<i<<std::setw(20);
            lines.push_back( ss.str()); ss.str(std::string());
        }   lines.push_back("]");
        return lines;
    }

    template<typename T>
    Lines_t tolines( std::string const& s, std::vector<T> const & v, Indices_t indices)
    {
        Lines_t lines;
        lines.push_back( tostr(s, "(size=", v.size(), ") [") );
        for( auto index : indices ) {
            lines.push_back( tostr(std::setw(6), index, std::setw(20), v[index]) );
        }   lines.push_back("]");
        return lines;
    }
 //---------------------------------------------------------------------------------------------------------------------
}// namespace mpi

#define STATIC_INFO_DECL                               \
    static                                             \
    std::string                                        \
    static_info                                        \
      ( std::string const& indent = std::string("\n")  \
      , std::string const& title  = std::string()      \
      )

#define STATIC_INFO_DEF(classname)  \
    std::string                     \
    classname::                     \
    static_info                     \
      ( std::string const& indent   \
      , std::string const& title    \
      )

#define INFO_DECL                                       \
    std::string                                         \
    info                                                \
      ( std::string const& indent = std::string("\n")   \
      , std::string const& title = std::string()        \
      ) const

#define INFO_DEF(classname)         \
    std::string                     \
    classname::                     \
    info                            \
      ( std::string const& indent   \
      , std::string const& title    \
      ) const

#include <stdint.h>
#include <limits.h>

#if SIZE_MAX == UCHAR_MAX
   #define MPI_SIZE_T MPI_UNSIGNED_CHAR
#elif SIZE_MAX == USHRT_MAX
   #define MPI_SIZE_T MPI_UNSIGNED_SHORT
#elif SIZE_MAX == UINT_MAX
   #define MPI_SIZE_T MPI_UNSIGNED
#elif SIZE_MAX == ULONG_MAX
   #define MPI_SIZE_T MPI_UNSIGNED_LONG
#elif SIZE_MAX == ULLONG_MAX
   #define MPI_SIZE_T MPI_UNSIGNED_LONG_LONG
#else
   #error "what is happening here?"
#endif

#endif // MPICTS_H
