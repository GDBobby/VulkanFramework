#include "EightWinds/Command/CommandPool.h"


namespace EWE{

    [[nodiscard]] CommandPool::CommandPool(LogicalDevice& logicalDevice, Queue& queue, bool auxilary)
        : logicalDevice{logicalDevice}, 
            queue{queue},
            flags{auxilary ? 
                static_cast<VkCommandPoolCreateFlags>(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT) 
                : static_cast<VkCommandPoolCreateFlags>(0)} 
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