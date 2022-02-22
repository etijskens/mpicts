#include "MessageData.h"

namespace mpi
{//------------------------------------------------------------------------------------------------
    void
    MessageData::
    getBuffer
      ( size_t nBytes
      )
    {
        messageBuffer_ = Buffer::getBuffer(nBytes);
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
