#include <iostream>

#include "MessageBuffer.h"

namespace mpi
{//------------------------------------------------------------------------------------------------
 // Implementation of class Buffer
 //-------------------------------------------------------------------------------------------------
    INFO_DEF(Buffer)
    {
        std::stringstream ss;
        ss<<indent<<"Buffer.info("<<title<<") ( ptr="<<(void*) pBytes_ // cast needed to avoid interpreting char* as string
                                           <<", nBytes="<< size()
                                           <<", useCount="<< useCount_
                                           <<" )";
        return ss.str();
    }

 //-------------------------------------------------------------------------------------------------
 // Implementation of class SharedBuffer
 //-------------------------------------------------------------------------------------------------
    INFO_DEF(SharedBuffer)
    {
        std::stringstream ss;
        ss<<indent<<"SharedBuffer.info("<<title<<") : ( ";
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
 // Implementation of class MessageBufferPool
 //-------------------------------------------------------------------------------------------------
    SharedBuffer
    MessageBufferPool::
    getBuffer(size_t nBytes)
    {
        auto iter = pool_.begin();
        for( ; iter != pool_.end(); ++iter )
        {
            if( iter->useCount_ > 0 )
                continue;
            if( iter->size() >= nBytes )
                break;
        }
        Buffer* pBuffer = nullptr;
        if( iter == pool_.end() )
        {// no large enough buffer available. Create one:
            pool_.push_back( Buffer(nBytes) );
            pBuffer = &pool_.back();
        } else {
            pBuffer = &(*iter);
        }
        return SharedBuffer(pBuffer);
    }

 //-------------------------------------------------------------------------------------------------
    INFO_DEF(MessageBufferPool)
    {
        std::stringstream ss;
        ss<<indent<<"MessageBufferPool.info("<<title<<"): ( ";
        if( pool_.size() ) {
            ss<<"size="<<pool_.size()<<" )";
            for( size_t i=0; i < pool_.size(); ++i ) {
               ss<<pool_[i].info(indent + "  ", std::string("i=") + std::to_string(i));
            }
        } else {
            ss <<"empty )";
        }
        return ss.str();
    }

 //-------------------------------------------------------------------------------------------------
}// namespace mpi
