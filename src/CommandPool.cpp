#include "EightWinds/Command/CommandPool.h"


namespace EWE{

    [[nodiscard]] CommandPool::CommandPool(LogicalDevice& logicalDevice, Queue& queue, VkCommandPoolCreateFlags createFlags)
        : logicalDevice{logicalDevice}, 
            queue{queue},
            flags{ createFlags }
        {
            VkCommandPoolCreateInfo commandPoolCreateInfo{};
            commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            commandPoolCreateInfo.pNext = nullptr;
            commandPoolCreateInfo.queueFamilyIndex = queue.family.index;
            commandPoolCreateInfo.flags = flags;
            EWE_VK(vkCreateCommandPool,
                logicalDevice.device, 
                &commandPoolCreateInfo, 
                nullptr, 
                &commandPool
            );
    }    
}