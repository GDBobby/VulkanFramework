#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/RenderGraph/GPUTask.h"

namespace EWE{
    struct TaskBridge{
        LogicalDevice& device;
        GPUTask& lhs;
        GPUTask& rhs; //rhs depends on lhs
        Queue& queue; //equal to rhs queue

        std::vector<VkBufferMemoryBarrier2> bufferBarriers{};
        std::vector<VkImageMemoryBarrier2> imageBarriers{};

        void CreateBarriers();
    };
}