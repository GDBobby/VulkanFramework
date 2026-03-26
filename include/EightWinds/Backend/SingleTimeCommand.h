#pragma once
#include "EightWinds/Queue.h"
#include "EightWinds/CommandPool.h"
#include "EightWinds/CommandBuffer.h"
#include "EightWinds/Backend/Semaphore.h"

namespace EWE{


    struct SingleTimeCommand {
        CommandPool* cmdPool;
        CommandBuffer cmdBuf;
        TimelineSemaphore* semaphore;
        [[nodiscard]] explicit SingleTimeCommand(CommandPool* _cmdPool, TimelineSemaphore* _semaphore) 
        : cmdPool { _cmdPool }, 
            cmdBuf{ cmdPool->AllocateCommand(VK_COMMAND_BUFFER_LEVEL_PRIMARY) },
            semaphore{_semaphore}
        {} //construct the commandbuffer here
        SingleTimeCommand(SingleTimeCommand const& copySrc) = delete;
        SingleTimeCommand& operator=(SingleTimeCommand const& copySrc) = delete;
        SingleTimeCommand(SingleTimeCommand&& moveSrc) noexcept;
        SingleTimeCommand& operator=(SingleTimeCommand&& moveSrc);
    };
}