#pragma once

#include "EightWinds/VulkanHeader.h"

#include "EightWinds/Command/CommandBuffer.h"

namespace EWE{
    struct CommandBuffer;

    namespace Backend{
        struct SubmitInfo {
            std::vector<VkSemaphoreSubmitInfo> waitSemaphores;
            std::vector<VkSemaphoreSubmitInfo> signalSemaphores;

            std::vector<CommandBuffer*> cmdBuffers;

            //dont directly deal with commands, VkCommandBufferSubmitInfo isn't for game development
            std::vector<VkCommandBufferSubmitInfo> commandInfos;

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