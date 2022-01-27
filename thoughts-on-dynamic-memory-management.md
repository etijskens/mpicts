### Thoughts on dynamic memory management

Now that can communicate contiguous and non-contiguous data, we face the issue that

1. the number of messages is not a priori known,
2. neither is the size of a message.

Currently, the number of message headers is fixed, as well as the total size of the message
buffer.

We must build in some way of dynamic memory management to accommodate a variable number 
of message headers, and messages of arbitrary size. 

Currently, the 
