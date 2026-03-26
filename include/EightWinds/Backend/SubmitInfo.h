#pragma once

#include "EightWinds/VulkanHeader.h"

#include "EightWinds/CommandBuffer.h"
#include <vulkan/vulkan_core.h>

namespace EWE{
    struct CommandBuffer;

    namespace Backend{
        struct SubmitInfo {
        private:
            std::vector<VkSemaphoreSubmitInfo> internal_waitSemaphores;
            std::vector<CommandBuffer*> cmdBuffers;
            //dont directly deal with commands, VkCommandBufferSubmitInfo isn't for game development
            std::vector<VkCommandBufferSubmitInfo> commandInfos;

            void ApplyWaitSemaphores();
        public:
            std::vector<VkSemaphoreSubmitInfo*> waitSemaphores;
            std::vector<VkSemaphoreSubmitInfo> signalSemaphores;

            void AddCommandBuffer(CommandBuffer& cmdBuf);

            void WaitOnPrevious(SubmitInfo& previous);

            VkSubmitInfo2 Expand();

            VkSubmitInfo2 ExpandWithoutSignal();
        };
    }
}

            /*
            VkStructureType                     sType;
            const void*                         pNext;
            VkSubmitFlags                       flags;
            uint32_t                            waitSemaphoreInfoCount;
            const VkSemaphoreSubmitInfo*        pWaitSemaphoreInfos;
            uint32_t                            commandBufferInfoCount;
            const VkCommandBufferSubmitInfo*    pCommandBufferInfos;
            uint32_t                            signalSemaphoreInfoCount;
            const VkSemaphoreSubmitInfo*        pSignalSemaphoreInfos;
            */