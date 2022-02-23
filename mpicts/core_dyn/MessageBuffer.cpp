#include <iostream>

#include "MessageBuffer.h"

namespace mpi
{//------------------------------------------------------------------------------------------------
 // Implementation of class MessageBuffer
 //-------------------------------------------------------------------------------------------------
    INFO_DEF(MessageBuffer)
    {
        std::stringstream ss;
        ss<<indent<<"MessageBuffer.info("<<title<<") : ( ";
        ss<<  "ptr="<<(void*)pBuffer_ // cast needed to avoid interpreting char* as string
          <<", size="<<nBytes_
          <<" )";
        return ss.str();
    }

 //-------------------------------------------------------------------------------------------------
}// namespace mpi
