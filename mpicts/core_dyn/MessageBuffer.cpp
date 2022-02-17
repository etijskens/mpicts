#include <iostream>

#include "MessageBuffer.h"

namespace mpi
{//------------------------------------------------------------------------------------------------
 // Implementation of class Buffer
 //-------------------------------------------------------------------------------------------------
    std::string
    Buffer::
    info( std::string const& s ) const
    {
        std::stringstream ss;
        ss  <<  "Buffer      : "
            <<"\n    raw ptr : "<< (void*) pBytes_ // cast needed to avoid interpreting char* as string
            <<"\n    nBytes  : "<< size()
            <<"\n    useCount: "<< useCount_
            <<std::endl;
        return ss.str();
    }

 //-------------------------------------------------------------------------------------------------
 // Implementation of class SharedBuffer
 //-------------------------------------------------------------------------------------------------
    std::string
    SharedBuffer::
    info( std::string const& s ) const
    {
        std::stringstream ss;
        ss  <<  "SharedBuffer: "<<s;
        if(pBuffer_) {
        ss  <<"\n    raw ptr : "<< (void*)(pBuffer_->pBytes_) // cast needed to avoid interpreting char* as string
            <<"\n    nBytes  : "<< pBuffer_->size()
            <<"\n    useCount: "<< pBuffer_->useCount_;
        } else {
        ss  <<"\n    (empty)";
        }
        ss<<std::endl;
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
    std::string
    MessageBufferPool::
    info(std::string const& s) const
    {
        std::stringstream ss;
        ss  <<"MessageBufferPool info: "<<s
            <<"\n    size    : "<< pool_.size();
        for( size_t i=0; i < pool_.size(); ++i )
        {
          ss<<"\n  pool["<<i<<"]"
            <<"\n    raw ptr :"<< (void*)(pool_[i].pBytes_) // cast needed to avoid interpreting char* as string
            <<"\n    nBytes  :"<< pool_[i].size()
            <<"\n    useCount:"<< pool_[i].useCount_
            <<std::endl;
        }
        return ss.str();
    }

 //-------------------------------------------------------------------------------------------------
}// namespace mpi
