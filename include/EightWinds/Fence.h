#pragma once
#include "EightWinds/VulkanHeader.h"
#include "Semaphore.h"

namespace EWE{
    struct Fence {
        VkFence vkFence{ VK_NULL_HANDLE };
        bool inUse{ false };
        bool submitted{ false };
        std::vector<Semaphore*> waitSemaphores{};
#if DEBUGGING_FENCES
        std::vector<std::string> log{};
#endif

        //its up to the calling function to unlock the mutex
        bool CheckReturn(uint64_t time);
    };
}