#pragma once
#include "EightWinds/VulkanHeader.h"
#include "EightWinds/Backend/Semaphore.h"

namespace EWE{
    struct Fence {
        LogicalDevice& logicalDevice;
        VkFence vkFence{ VK_NULL_HANDLE };

        [[nodiscard]] explicit Fence(LogicalDevice& logicalDevice, VkFenceCreateInfo const& fenceCreateInfo);
        ~Fence();
        operator VkFence() const {
            return vkFence;
        }

        bool Wait(uint64_t wait_timer) {

        }

        bool inUse{ false };
        bool submitted{ false };
        std::vector<Semaphore*> waitSemaphores{};
#if DEBUGGING_FENCES
        std::vector<std::string> log{};
#endif

        //its up to the calling function to unlock the mutex
        bool CheckReturn(uint64_t time);

        
#if EWE_DEBUG_NAMING
        std::string debugName;
        void SetName(std::string_view name);
#endif
    };
}