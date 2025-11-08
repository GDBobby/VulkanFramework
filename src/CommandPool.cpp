#include "EightWinds/CommandPool.h"


namespace EWE{

    [[nodiscard]] CommandPool::CommandPool(LogicalDevice& logicalDevice, Queue& queue, bool auxilary)
        : logicalDevice{logicalDevice}, queue{queue} {
            VkCommandPoolCreateInfo commandPoolCreateInfo{};
            commandPoolCreateInfo.queueFamilyIndex = queue.family.index;
            commandPoolCreateInfo.flags = 
                auxilary ? 
                VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT 
                : 0
            ;
            vkCreateCommandPool(
                logicalDevice.device, 
                &commandPoolCreateInfo, 
                nullptr, 
                &commandPool
            );
    }    
}