#pragma once
#include "EightWinds/VulkanHeader.h"
#include "Semaphore.h"

namespace EWE{
    struct Fence {
        LogicalDevice& logicalDevice;

        [[nodiscard]] explicit Fence(LogicalDevice& logicalDevice) : logicalDevice{logicalDevice} {}

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