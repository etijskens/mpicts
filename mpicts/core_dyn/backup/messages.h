#ifndef MESSAGES_H
#define MESSAGES_H

#include "mpicts.h"
#include "memcpy_able.h"

#include <vector>


namespace mpi
{//------------------------------------------------------------------------------------------------
    struct MessageHeader
    {
          MessageHandlerKey_t messageHandlerKey;  // MessageHandler key of the message
          size_t  size; // message size in bytes
          int     from; // the source MPI rank of the message
          int     to;   // the destination MPI rank of the message
    };
 //-------------------------------------------------------------------------------------------------
    class MessageItemBase
 //-------------------------------------------------------------------------------------------------
    {
    public:
        virtual ~MessageItemBase() {}
    // write the message item to dst
        virtual void write(void*& dst) const = 0;
    // read the message item from src
        virtual void read (void*& src)       = 0;
    // get the size of the message item (in bytes)
        virtual size_t messageSize() const = 0;
    };

 //-------------------------------------------------------------------------------------------------
    template <typename T>
    class MessageItem : public MessageItemBase
 //-------------------------------------------------------------------------------------------------
    {
        static const bool _debug_ = true; // write debug output or not

     // data members
        T* ptrT_;

    public:
        MessageItem(T& t) // Contiguous
          : ptrT_(&t)
          {}

        ~MessageItem()
        {
            if constexpr(::mpi::_debug_ && _debug_) {
                prdbg(tostr("~MessageItem<T=", typeid(T).name(), ">() : this=", this));
            }
        }

     // Write the content of ptrT_ to dst
        virtual void write( void*& dst ) const
       {
            if constexpr(::mpi::_debug_ && _debug_) {
                prdbg( tostr("MessageItem<T=", typeid(T).name(), ">::write()") );
            }
            ::mpi::write( *ptrT_, dst );
        }

     // Read the content of ptrT_ from src
        virtual void read(void*& src)
        {
            ::mpi::read( *ptrT_, src );

            if constexpr(::mpi::_debug_ && _debug_) {
                prdbg( tostr("MessageItem<T=", typeid(T).name(), ">::read()") );
            }
        }

     // Size that *ptrT_ will occupy in a message, in bytes
        virtual size_t messageSize() const {
            return ::mpi::messageSize(*ptrT_);
        }
    };
 //-------------------------------------------------------------------------------------------------
 // Specialisation
    template <>
    class MessageItem<ParticleContainer> : public MessageItemBase
 //-------------------------------------------------------------------------------------------------
    {
        static const bool _debug_ = true; // write debug output or not

    private: // data members
        ParticleContainer* ptr_pc_;
        Indices_t indices_;
        bool move_;
    public:
     // ctor
        MessageItem
          ( ParticleContainer& pc
          )
          : ptr_pc_(&pc)
        {}

     // Set the selection of particles ane whether to move or copy them,
        void
        select
          ( Indices_t& indices // list of particles to be moved/copied.
          , bool move          // choose between moving or copying the particles to the other proces
          )
        {
            indices_ = indices; // make a copy
            move_ = move;
        }

     // indices accessor
        Indices_t const& indices() const { return indices_; }
        Indices_t      & indices()       { return indices_; }

     // dtor
        ~MessageItem()
        {
            if constexpr(::mpi::_debug_ && _debug_) {
                prdbg(tostr("~MessageItem<ParticleContainer>()"));
            }
        }

     // Write the number of selected particles to the MessageBuffer. This is all the reader has to know
     // (how many new particles must be created). Remove the particle from the ParticleContainer if requested.
        virtual
        void
        write
          ( void*& dst // pointer in the MessageBuffer where this item must be written.
                       // Is advanced by the number of bytes written on return
          ) const
       {
            if constexpr(::mpi::_debug_ && _debug_) {
                Lines_t lines = tolines("indices ", indices());
                lines.push_back( tostr("move = ", move_, (move_ ? "(particles are deleted)" : "(particles are copied)")) );
                prdbg( tostr("MessageItem<ParticleContainer>::write(dst)"), lines );
            }

            size_t nParticles = indices_.size();
            ::mpi::write( nParticles, dst );
            if( move_ )
            {// Remove the particles from the ParticleContainer
                for( auto index : indices_ )
                    ptr_pc_->remove(index);
            }
        }

     // Read the number of selected particles and create as many new particles in the ParticleContainer.
        virtual void read(void*& src)
        {
            Index_t n;
            ::mpi::read( n, src );
         // create n new particles
            indices_.resize(n);
            for( size_t i = 0; i < n; ++i )
                indices_[i] = ptr_pc_->add();

            if constexpr(::mpi::_debug_ && _debug_) {
                prdbg( tostr("MessageItem<ParticleContainer>::read(src)"), tolines("new indices", indices_) );
            }
        }

     // The number of byte that this MessageItem will occupy in a MessageBuffer
        virtual size_t messageSize() const {
            return sizeof(size_t);
        }
   };
 //-------------------------------------------------------------------------------------------------
 // Specialisation
    template <typename T>
    class MessageItem<ParticleArray<T>> : public MessageItemBase
 //-------------------------------------------------------------------------------------------------
    {
        static const bool _debug_ = true; // write debug output or not

    private: // data members
        ParticleArray<T>* ptr_pa_;
        MessageItem<ParticleContainer>* ptr_pc_message_item_;

    public:
     // ctor
        MessageItem
          ( ParticleArray<T>& pa
          , MessageItemBase* ptr_pc_message_item
          )
          : ptr_pa_(&pa)
          , ptr_pc_message_item_( dynamic_cast<MessageItem<ParticleContainer>*>(ptr_pc_message_item) )
        {}

     // dtor
        ~MessageItem()
        {
            if constexpr(::mpi::_debug_ && _debug_) {
                prdbg(tostr("~MessageItem<ParticleArray<T=", typeid(T).name(), ">>()"));
            }
        }

     // Write the selected array elements to the MessageBuffer
        virtual
        void
        write
          ( void*& dst // pointer in the MessageBuffer where this item must be written.
                       // Is advanced by the number of bytes written on return
          ) const
       {
            if constexpr(::mpi::_debug_ && _debug_) {
                prdbg( tostr("MessageItem<ParticleArray<T=", typeid(T).name(), ">>::write(dst)")
                     , tolines(ptr_pa_->name(), *ptr_pa_, ptr_pc_message_item_->indices() )
                     );
            }
            for( auto index : ptr_pc_message_item_->indices() )
                ::mpi::write( (*ptr_pa_)[index], dst );
        }

     // Read the selected particles from src
        virtual
        void
        read
          ( void*& src // pointer in the MessageBuffer where this item must be written.
                       // Is advanced by the number of bytes written on return
          )
        {
            for( auto index : ptr_pc_message_item_->indices() )
                ::mpi::read( (*ptr_pa_)[index], src );

            if constexpr(::mpi::_debug_ && _debug_) {
                prdbg( tostr("MessageItem<ParticleArray<T=", typeid(T).name(), ">>::read(src)")
                     , tolines(ptr_pa_->name(), *ptr_pa_, ptr_pc_message_item_->indices() )
                     );
            }
        }

     // The number of bytes that this MessageItem will occupy in a MessageBuffer
        virtual size_t messageSize() const {
            return ptr_pc_message_item_->indices().size() * sizeof(T);
        }
    };

 //-------------------------------------------------------------------------------------------------
    template
      < size_t WordSize // Pad the nBytes parameter to the next WordSize boundary
      , size_t Unit=1   // Express the result as a multiple of Unit bytes (Unit must be a divisor of WordSize).
      >
    size_t              // the result
    padBytes            // padBytes<8>(5)   -> 8 : the next 8 byte boundary of 5 (bytes) is 8 (bytes)
                        // padBytes<8,4>(5) -> 2 : the next 8 byte boundary of 5 (bytes) is 2 4-byte words (= 8 bytes)
                        // padBytes<8,2>(5) -> 8 : the next 8 byte boundary of 5 (bytes) is 1 8-byte word (= 8 bytes)
      ( size_t nBytes   // the number of bytes to be padded.
      )
    {
        static_assert(WordSize % Unit == 0); // Unit must be a divisor of WordSize.
        return ((nBytes + WordSize - 1) / WordSize) * (WordSize / Unit);
    }

 //-------------------------------------------------------------------------------------------------
    class MessageBuffer
 //-------------------------------------------------------------------------------------------------
    {
        typedef Index_t Word_t; // The size of the buffer is internally expressed in terms of Word_t blocks.

        Word_t * pBuffer_;
        size_t size_;
    public:
        MessageBuffer() // Construct empty MessageBuffer object (without allocated memory).
          : pBuffer_(nullptr), size_(0)
        {}
        
        ~MessageBuffer()
        {
            free();
        }
        
        void* // void pointer to the buffer 
        data()
        {
            return static_cast<void*>(pBuffer_);
        }
        
        void 
        allocate(size_t nBytes) // allocate a buffer that can accomodate a message of nBytes bytes.
        {        
            size_t nWords = 0;//padBytes<sizeof(Word_t),sizeof(Word_t)>(nBytes);
             // now nBytes <= nWords * sizeof(Word_t) > nBytes + sizeof(Word_t)
            if( nWords <= size_ )
            {// buffer is large enough, nothing to do.
            } else
            {// current memory allocation is too small. Release it: 
                free();
             // allocate large enough memory space.
                pBuffer_ = new Word_t[nWords];
                size_ = nWords;
            }
        }
        
        void free() // free the allocated memory, if any.
        {
            if( pBuffer_ ) {
                delete[] pBuffer_;
                pBuffer_ = nullptr;
                size_ = 0;
            }
        }
    };
    
 //-------------------------------------------------------------------------------------------------
    class MessageBody 
 // provides member functions to write, read and obtain the size of a message.
 //-------------------------------------------------------------------------------------------------
    {
    private:
        static bool const _debug_ = true;

        std::vector<MessageItemBase*> msgItems_;   // Ordered list of MessageItem<T>* objects.
        MessageBuffer                 msgBuffer_;  //
        MessageHeader*                pMsgHeader_; // to access message header properties from the message body.

    public:
        MessageBody() // Construct empty MessageBody, the full construction is done by a MessageCollection object
          : pMsgHeader_(nullptr)
        {}
        
        ~MessageBody();

     // Add an item to the message
        template<typename T>
        MessageItem<T>* // Return the constructed MessageItem. Occasionally needed.
        push_back
          ( T& t // object to incorporate in the message
          )
        {
            MessageItem<T>* p = new MessageItem<T>(t);
            msgItems_.push_back(p);
            return p;
        }

     // Add an item to the message (behaves as FIFO)
        template<typename T>
        MessageItem<T>* // Return the constructed MessageItem. Occasionally needed.
        push_back
          ( T& t // object to incorporate in the message
          , MessageItemBase* ptr_mi // a MessageItem that contains information needed by this MessageItem
                                    // e.g. the ParticleArray MessageItem needs a ParticleContainer MessageItem.
          )
        {
            MessageItem<T>* p = new MessageItem<T>(t, ptr_mi);
            msgItems_.push_back(p);
            return p;
        }
        
     // Write header
        void writeHeader()

     // Write the message to ptr in the MessageBuffer
        void write(void*& ptr) const;
     // same but use message buffer owned by *this.
        void write() const;

     // Read the message from ptr in the MessageBuffer
        void read(void*& ptr);
     // same but use message buffer owned by *this.
        void read();

     // Compute The number of bytes the message will occupy in the message buffer
        size_t messageSize() const;

     // Operations for sending ////////////////////////////////////////////////////////////////////////
     // Make sure the msg_buffer_ is allocated and large enough.
        void checkMessageBuffer(); // for sending, the size of the MessageBuffer is computed from the message
         // todo check if this is working. is there enough information to compute the size of the message in case of a particle container?


     // check buffer, write buffer and send.
        void sendMessage() const; // const?

     // Operations for receiving ////////////////////////////////////////////////////////////////////////
        void checkMessageBuffer // for receiving
          ( size_t messageSize // typically obtained from the corresponding MessageHeader.
          );

        void receiveMessage() const; // const?
    };
 //-------------------------------------------------------------------------------------------------
    class MessageCollection
    {
        std::vector<MessageHeader> msgHeaders_;
        std::vector<MessageBody>   msgBodies_;
        int rank_;
    public:
        MessageCollection(int rank=-1) // create a MessageCollection for rank `rank`.
          : rank_(rank)
        {}

        int rank() const { return rank_; } // Return the rank of this MessageCollection

        size_t size() const // Return the size of this MessageCollection, i.e. the number of messages.
        {
            return msgHeaders_.size();
        }

        MessageBody&
        addMessage                      // add empty message to this messageCollection
          ( int to_rank                 // the destination of the message
          , MessageHandler const& messageHandler   // the messageHandler
          );

        void resize   // resize the MessageCollection (the number of messages to be accounted for.
          ( size_t sz // the new size
          )
        {
            msgHeaders_.resize(sz);
            msgBodies_ .resize(sz);
            for( size_t i=0; i < sz; ++i )
            {
                msgBodies_[i].pMsgHeader_ = &msgHeaders_[i];
            }
         // todo fix issue below if the number of messages is not constant over the entire simulation.
            if( this->size() != 0 )
            {// so far the number of message has been constant over the simulation.
             // we probably need copy ctors for the MessageHeaders and MessageBodies
                std::cout
                    <<"\nWARNING: resizing a non-empty MessageCollection: Untested and probably not correct."
                    <<std::endl;
            }
        }

        Index_t* ptrToMessageHeaders()
        {
            return msgHeaders_.data();
        }

        int sizeofMessageHeaders()
        {
            return size() * sizeof(MessageHeader);
        }

        MessageHeader&
        header       // Access the MessageHeader of the i-th message in this MessageCollection
          ( size_t i // index of a message in this MessageCollection
          )
        {
            return msgHeaders_[i];
        }

        MessageBody&
        body         // Access the MessageBody of the i-th message in this MessageCollection
          ( size_t i // index of a message in this MessageCollection
          )
        {
            return msgBodies_[i];
        }

        void
        checkSendBuffer // Make sure that the MessageBuffer of the i-th message is large enough.
                        // Its size is computed by the MessageBody itself.
                        // This is for messages that must be sent.
          ( size_t i    // index of the message in the MessageCollection
          )
        {
            body(i).checkMessageBuffer();
        }

        void
        checkReceiveBuffer // Make sure that the MessageBuffer of the i-th message is large enough.
                           // Its size must be (at least) the size specified by the header.
                           // This is for messages that must be received.
          ( size_t i       // index of the message in the MessageCollection
          )
        {
            headerSize = header(i).size; // in Index_t words
            body(i).checkMessageBuffer( headerSize * sizeof(Index_t) );
        }
    };

 //-------------------------------------------------------------------------------------------------
    class Messages
    {
        std::vector<MessageCollection> messageCollections_; // we want a MessageCollection for every rank
        std::vector<Index_t> nMessages_;
    public:
        Messages()
        {
            if( mpi::size > 1 )
            {// Construct a MessageCollection for each rank
                for( int rank=0; rank < mpi::size; ++rank)
                    messageCollections_.push_back( MessageCollection(rank) );
                nMessages_.resize(mpi::size);
            }
        }

        MessageCollection& // The MessageCollection returned
        messageCollection  // Access the MessageCollection for rank `rank`
          ( int rank       // the requested rank
          )
        {
            return messageCollections_[rank];
        }

     // send and receive all the messages.
        void communicate();
    };
 //------------------------------------------------------------------------------------------------
    extern Messages theMessages;
 //------------------------------------------------------------------------------------------------
}// namespace mpi

#endif // MESSAGES_H
