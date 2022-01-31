### Thoughts on dynamic memory management

Now that can communicate contiguous and non-contiguous data, we face the issue that

1. the number of messages is not a priori known,
2. neither is the size of a message.

Currently, the number of message headers is fixed, as well as the total size of the message
buffer.

We must build in some way of dynamic memory management to accommodate a variable number 
of message headers, and messages of arbitrary size. 

Currently, the MessageBuffer allocates a chunk of memory that is used for both the headers 
and the messages. 

program flow:

1. every process creates message for some other processes.
   1. create a message header
   2. create a message (collection of MessageItems)
   3. create a message buffer.
   
   This task is entirely local and does not involve communication. 

2. Every process sends all its headers to all other processes
   1. first it has to send the number of messages it has created to all other processes, 
      so they can provide space for the message headers. The most practical way is that 
      there is a message header collection for each process   
3. Every process sends its messages to their destination
4. Every process checks the message headers from all other processes and receives the
   corresponding message iff it its destination is the current process.
   1. it must first allocate memory for message (the size in the received message header is
      correct, but the message pointer obviously not. -> maybe we should not transfer 
      the message pointer...)

The latter comment means that the message pointer should not be part of the message header 
