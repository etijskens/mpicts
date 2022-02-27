#include "MessageData.h"

namespace mpi
{//------------------------------------------------------------------------------------------------
    void
    MessageData::
    allocateBuffer()
    {
        size_t nBytes = messageHeader_.size();
        messageBuffer_.alloc( nBytes );
//        prdbg(tostr("MessageData::allocateBuffer(", nBytes, ")", info()));
    }


 //------------------------------------------------------------------------------------------------
    INFO_DEF(MessageData)
    {
        std::stringstream ss;
        ss<<indent<<"MessageData.info("<<title<<") :"
                  <<messageHeader_.info(indent + "  ")
                  <<messageBuffer_.info(indent + "  ")
                  ;
        return ss.str();
    }

 //------------------------------------------------------------------------------------------------
}// namespace mpi
