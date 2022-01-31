#ifndef MESSAGECONTAINER_H
#define MESSAGECONTAINER_H

#include <vector>
#include <memory>

#include "mpicts.h"
#include "MessageHeader.h"
#include "Message.h"


namespace mpi
{//------------------------------------------------------------------------------------------------
    typedef std::shared_ptr<Index_t> msgbfr_ptr_t;
 //------------------------------------------------------------------------------------------------
    class ProcessMessageContainer
    {
        std::vector<MessageHeader> headers_;
        std::vector<msgbfr_ptr_t> msg_buffers_;
    public:
        ProcessMessageContainer();
    };
 //------------------------------------------------------------------------------------------------
    class GlobalMessageContainer
    {
        std::vector<ProcessMessageContainer> processMessageContainers_;
    public:
        GlobalMessageContainer();

    private:
    };
 //------------------------------------------------------------------------------------------------

}// namespace mpi

#endif // MESSAGECONTAINER_H
