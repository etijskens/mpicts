#ifndef EXTRAMESSAGEDATA_H
#define EXTRAMESSAGEDATA_H


namespace mpi
{//-------------------------------------------------------------------------------------------------
    struct ExtraMessageData{};
 // Base class for adding extra message data.
 // Extra message data are needed when A MessageHandler is responsible for sending several messages
 // to different destinations. E.g a ParticleContainer may need to move and/or copy particles to
 // several neighbouring processess (not even necessarily nearest neighours). The extra data
 // distinguishes the messages
 // The MessageHandler is responsible for the memory management and the handling of these objects.
 // The Message objects just store a pointer to the ExtraMessageData.
 // If there are no ExtraMessageData, the pointer is a nullptr.
 //-------------------------------------------------------------------------------------------------
}// namespace mpi
#endif // EXTRAMESSAGEDATA_H
