#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/RenderGraph/GPUTask.h"

namespace EWE{
    struct TaskBridge{
        LogicalDevice& logicalDevice;
        GPUTask& lhs;
        GPUTask& rhs; //rhs depends on lhs

        [[nodiscard]] explicit TaskBridge(GPUTask& lhs, GPUTask& rhs) noexcept
            : logicalDevice{lhs.logicalDevice},
            lhs{lhs},
            rhs{rhs}
        {
		    dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		    dependencyInfo.pNext = nullptr;
            dependencyInfo.dependencyFlags = 0; //might need to fine tune this
        }

        std::vector<VkBufferMemoryBarrier2> bufferBarriers{};
        std::vector<VkImageMemoryBarrier2> imageBarriers{};
		VkDependencyInfo dependencyInfo{};

        void RecreateBarriers();

        void Submit(CommandBuffer& cmdBuf);
    };
}