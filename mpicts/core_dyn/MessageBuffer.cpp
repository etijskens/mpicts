#include <iostream>

#include "MessageBuffer.h"

namespace mpi
{//------------------------------------------------------------------------------------------------
 // Implementation of class Buffer
 //-------------------------------------------------------------------------------------------------
    std::vector<Buffer> Buffer::theMessageBufferPool;

    MessageBuffer
    Buffer::
    getBuffer(size_t nBytes)
    {
        auto iter = theMessageBufferPool.begin();
        for( ; iter != theMessageBufferPool.end(); ++iter )
        {
            if( iter->useCount_ > 0 )
                continue;
            if( iter->size() >= nBytes )
                break;
        }
        Buffer* pBuffer = nullptr;
        if( iter == theMessageBufferPool.end() )
        {// no large enough buffer available. Create one:
            theMessageBufferPool.push_back( Buffer(nBytes) );
            pBuffer = &theMessageBufferPool.back();
        } else {
            pBuffer = &(*iter);
        }
        return MessageBuffer(pBuffer);
    }

    INFO_DEF(Buffer)
    {
        std::stringstream ss;
        ss<<indent<<"Buffer.info("<<title<<") ( ptr="<<(void*) pBytes_ // cast needed to avoid interpreting char* as string
                                           <<", nBytes="<< size()
                                           <<", useCount="<< useCount_
                                           <<" )";
        return ss.str();
    }
    
    STATIC_INFO_DEF(Buffer)
    {
        std::stringstream ss;
        ss<<indent<<"theMessageBufferPool.info("<<title<<") :";
        if( theMessageBufferPool.size() ) {
            for( size_t i=0; i < theMessageBufferPool.size(); ++i ) {
               ss<<theMessageBufferPool[i].info( indent + "  ", tostr("i=", i, "/", theMessageBufferPool.size()) );
            }
        } else {
            ss<<indent<<"  ( empty )";
        }
        return ss.str();
    }
 //-------------------------------------------------------------------------------------------------
 // Implementation of class MessageBuffer
 //-------------------------------------------------------------------------------------------------
    INFO_DEF(MessageBuffer)
    {
        std::stringstream ss;
        ss<<indent<<"MessageBuffer.info("<<title<<") : ( ";
        if(pBuffer_) {
          ss<<  "ptr="<<(void*)(pBuffer_->pBytes_) // cast needed to avoid interpreting char* as string
            <<", size="<<       pBuffer_->size()
            <<", useCount="<<   pBuffer_->useCount_
            ;
        } else {
          ss<<"empty";
        }
        ss<<" )";
        return ss.str();
    }

 //-------------------------------------------------------------------------------------------------
}// namespace mpi
