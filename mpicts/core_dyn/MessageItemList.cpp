#include "MessageItemList.h"

namespace mpi
{
 //-------------------------------------------------------------------------------------------------
 // Implementation of class MessageItemList
 //-------------------------------------------------------------------------------------------------
    MessageItemList::
    ~MessageItemList()
    {
        if constexpr(::mpi::_debug_ && _debug_)
            prdbg("~MessageItemList() : deleting MessageItems");
        for( auto p : list_) {
            delete p;
        }
        if constexpr(::mpi::_debug_ && _debug_)
            prdbg("~MessageItemList() : MessageItems deleted.");
    }

    void
    MessageItemList::
    write
      ( void*& ptr // pointer where the message should be written to.
      ) const
    {
        MessageItemBase * const * pBegin = &list_[0];
        MessageItemBase * const * pEnd   = pBegin + list_.size();
        for( MessageItemBase * const * p = pBegin; p < pEnd; ++p) {
            (*p)->write(ptr);
        }
    }

    void
    MessageItemList::
    read
      ( void*& ptr // pointer where the message should be read from.
      )
    {
        MessageItemBase ** pBegin = &list_[0];
        MessageItemBase ** pEnd   = pBegin + list_.size();
        for( MessageItemBase ** p = pBegin; p < pEnd; ++p) {
            (*p)->read(ptr);
        }
    }

    size_t // the number of bytes the mesage occupies in the MessageBuffer
    MessageItemList::
    computeBufferSize() const
    {
        size_t sz = 0;
        for( auto pItem : list_) {
            sz += pItem->computeByteSize();
        }
        return sz;
    }

//    Lines_t
//    MessageItemList::
//    debug_text() const
//    {
//        Lines_t lines;
//        lines.push_back("message :");
//        size_t i = 0;
//        MessageItemBase * const * pBegin = &list_[0];
//        MessageItemBase * const * pEnd   = pBegin + list_.size();
//        for( MessageItemBase * const * p = pBegin; p < pEnd; ++p) {
//            lines.push_back( tostr("MessagaItem ",i++) );
//            Lines_t lines_i = (*p)->debug_text();
//            lines.insert(lines.end(), lines_i.begin(), lines_i.end());
//        }
//        return lines;
//    }
 //-------------------------------------------------------------------------------------------------
}// namespace mpi