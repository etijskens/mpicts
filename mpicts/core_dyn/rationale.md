## Rationale of messages

### High level program flow
A `MessageHandler` object (derived from `MessageHandlerBase`) is created for each 
message to be sent. This object is responsible for creating a list of `MenuItem` objects 
that defines the data in the message and the order. The `MessageHandler` object is also 
capable of writing/reading the message to/from a buffer that is transmissed across mpi 
processes. 
The `MessageHandler` object is linked to a `Messages` object which takes care of the mpi
communication of all the messages defined by the `MessageHandler` objects.

### Low level program flow

1. Every MPI rank constructs its own messages (header and body)
2. The number of messages in each rank is broadcast, so that every rank know how many
   message there are in each rank. This is necessary to allocate space for their headers
3. every rank broadcasts its message headers, so that every rank knows the source rank, 
   the destination rank, the `MessageHandler`, and the size of every message. Every rank
   thus knows from which ranks it must receive messages and can reserve space for them.
4. every rank sends its own messages to their destination
5. every rank inspects all headers of the other ranks to see if it is the destination of 
   the corresponding message. If that is the case it 
   1. allocates a message buffer, 
   2. receives the message,
   3. decodes the message with the corresponding `MessageHandler`. 





### Classes

A message consists of a `MessageHeader` and a `MessageBody`   

#### class `MessageHeader`
The `MessageHeader` contains
- the **source** and **destination** of the message
- the **MessageHandlerKey** of the message. This identifies the `MessageHandler` 
  responsible for decoding and encoding the MessageBody
- the **size** of the message. The message size is computed by the `MessageHandler` 
  and stored in the message header to enable it to allocate a buffer for it.


#### class `MessageBody`
The MessageObject contains
- a list of `MessageItems` objects
- a `MessageHandlerKey` identifying the `MessageHandler`
- a message buffer which contains the actual bytes being transmissed over the
  network. 

#### class MessageCollection
The messages to be sent or received by a particular rank 
 
#### class `Messages`
Stores per rank:
- a list (`std::vector`) of all the message headers from that rank
- a list (`std::vector`) of the corresponding message bodies (but only if the destination 
  is the current rank)

#### class MessageHandlerBase and derived classes
Their responsability is to create the list of MessageItems, and to encode them for 
transmission in a buffer on the sender, and decode the transmitted buffer on the
receiver.

Since encoding and decoding of a message is happening on distinct MPI processes, 
the `MessageHandler` objects must exist on the writing (=sending) process and 
on all the reading processes (=receiving). The easiest way to fix this is to
create all `MessageHandler` objects on all ranks in the same order, so that 
the order of creation is a unique identifier for the `MessageHandler` across all
MPI processes.
