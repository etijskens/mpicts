#include "Message.h"

namespace mpi
{
 //-------------------------------------------------------------------------------------------------
 // Implementation of class Message
 //-------------------------------------------------------------------------------------------------
    Message::
    ~Message()
    {
        if constexpr(::mpi::_debug_ && _debug_)
            prdbg("~Message() : deleting MessageItems");
        for( auto p : coll_) {
            delete p;
        }
        if constexpr(::mpi::_debug_ && _debug_)
            prdbg("~Message() : MessageItems deleted.");
    }

    void
    Message::
    write
      ( void*& ptr // pointer where the message should be written to.
      ) const
    {
        MessageItemBase * const * pBegin = &coll_[0];
        MessageItemBase * const * pEnd   = pBegin + coll_.size();
        for( MessageItemBase * const * p = pBegin; p < pEnd; ++p) {
            (*p)->write(ptr);
        }
    }

    void
    Message::
    read
      ( void*& ptr // pointer where the message should be read from.
      )
    {
        MessageItemBase ** pBegin = &coll_[0];
        MessageItemBase ** pEnd   = pBegin + coll_.size();
        for( MessageItemBase ** p = pBegin; p < pEnd; ++p) {
            (*p)->read(ptr);
        }
    }

    size_t // the number of bytes the mesage occupies in the MessageBuffer
    Message::
    messageSize() const
    {
        size_t sz = 0;
        MessageItemBase * const * pBegin = &coll_[0];
        MessageItemBase * const * pEnd   = pBegin + coll_.size();
        for( MessageItemBase * const * p = pBegin; p < pEnd; ++p) {
            sz += (*p)->messageSize();
        }
        return sz;
    }

//    Lines_t
//    Message::
//    debug_text() const
//    {
//        Lines_t lines;
//        lines.push_back("message :");
//        size_t i = 0;
//        MessageItemBase * const * pBegin = &coll_[0];
//        MessageItemBase * const * pEnd   = pBegin + coll_.size();
//        for( MessageItemBase * const * p = pBegin; p < pEnd; ++p) {
//            lines.push_back( tostr("MessagaItem ",i++) );
//            Lines_t lines_i = (*p)->debug_text();
//            lines.insert(lines.end(), lines_i.begin(), lines_i.end());
//        }
//        return lines;
//    }
 //-------------------------------------------------------------------------------------------------
}// namespace mpi