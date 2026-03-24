#pragma once
#include "EightWinds/Queue.h"
#include "EightWinds/CommandPool.h"
#include "EightWinds/CommandBuffer.h"
#include "EightWinds/Backend/Semaphore.h"

namespace EWE{


    struct SingleTimeCommand {
        Queue::Type queueType; //i dont think this is necessary (2/12)
        CommandPool* cmdPool;
        CommandBuffer cmdBuf;
        TimelineSemaphore* semaphore;
        [[nodiscard]] explicit SingleTimeCommand(Queue::Type _queueType, CommandPool* _cmdPool, TimelineSemaphore* _semaphore) 
        : queueType{ _queueType }, 
            cmdPool { _cmdPool }, 
            cmdBuf{ cmdPool->AllocateCommand(VK_COMMAND_BUFFER_LEVEL_PRIMARY) },
            semaphore{_semaphore}
        {} //construct the commandbuffer here
        SingleTimeCommand(SingleTimeCommand const& copySrc) = delete;
        SingleTimeCommand& operator=(SingleTimeCommand const& copySrc) = delete;
        SingleTimeCommand(SingleTimeCommand&& moveSrc) noexcept;
        SingleTimeCommand& operator=(SingleTimeCommand&& moveSrc);
    };
}