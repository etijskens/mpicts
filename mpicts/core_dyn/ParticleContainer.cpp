#include "ParticleContainer.h"

namespace mpi
{//---------------------------------------------------------------------------------------------------------------------
 // Implementation of class PcMessageData
 //---------------------------------------------------------------------------------------------------------------------
    INFO_DEF(PcMessageData)
    {
        std::stringstream ss;

        ss<<indent<<"PcMessageData.info("<<title<<") :"
                  <<MessageData::info(indent +"  ")
//                  <<messageHeader_.info(indent + "  ")
//                  <<messageBuffer_.info(indent + "  ")
          <<indent<<"  "<<"indices=[size="<<indices().size()<<"][ "
                  ;
        for( auto index : indices() ) ss<<index<<" ";
        ss<<"]"
          <<indent<<"  "<<"mode="<<str(mode());

        return ss.str();
    }

 //---------------------------------------------------------------------------------------------------------------------
 // Implementation of class PcMessageHandler
 //---------------------------------------------------------------------------------------------------------------------
    PcMessageHandler::
    PcMessageHandler(ParticleContainer& pc)
      : pc_(pc)
    {
     // Add the particle container subset
     // The corresponding MessageItem will only write the number of indices.
        ptr_pc_message_item_ = messageItemList().push_back(pc_);
     // Add the arrays
     // todo: use list of names as argument and a suitable default.
        messageItemList().push_back(pc_.r, ptr_pc_message_item_);
        messageItemList().push_back(pc_.m, ptr_pc_message_item_);
    }

    void
    PcMessageHandler::
    addSendMessage
      ( int destination
      , Indices_t const & selection
      , Mode mode
      )
    {
        sendMessages_.push_back(new PcMessageData( mpi::rank, destination, this->key_, selection, mode ));
    }

    void
    PcMessageHandler::
    addRecvMessage(int src, size_t i)
    {
        recvMessages_.push_back(new PcMessageData(src, i));

        if constexpr( mpi::_debug_ && _debug_ )
            prdbg(recvMessages_.back()->info());
    }

 //---------------------------------------------------------------------------------------------------------------------
}// namespace mpi
