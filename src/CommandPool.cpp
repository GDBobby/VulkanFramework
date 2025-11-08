#include "EightWinds/CommandPool.h"


namespace EWE{

    [[nodiscard]] CommandPool::CommandPool(LogicalDevice& logicalDevice, Queue& queue, bool auxilary)
        : logicalDevice{logicalDevice}, queue{queue} {
            vkCommandPoolCreateInfo commandPoolCreateInfo{};
            commandPoolCreateInfo.queueFamilyIndex = queue.family.index;
            commandPoolCreateInfo.flags = 
                auxilary ? 
                vkCommandPoolCreateFlagBits::eResetCommandBuffer | vkCommandPoolCreateFlagBits::eTransient 
                : vkCommandPoolCreateFlags{};
            logicalDevice.vkDevice.createCommandPool(commandPoolCreateInfo, nullptr);
    }    
}