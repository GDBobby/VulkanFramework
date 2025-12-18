#include "EightWinds/Command/CommandPool.h"

#include "EightWinds/Command/CommandBuffer.h"

#include <array>

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
    PerFlight<CommandBuffer> CommandPool::AllocateCommandsPerFlight(VkCommandBufferLevel buffer_level) {
        allocatedBuffers += max_frames_in_flight;

        std::array<VkCommandBuffer, max_frames_in_flight> raw_data;
        VkCommandBufferAllocateInfo allocInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = commandPool,
            .level = buffer_level,
            .commandBufferCount = max_frames_in_flight
        };
        EWE_VK(vkAllocateCommandBuffers, logicalDevice, &allocInfo, raw_data.data());
        return PerFlight<CommandBuffer>(
            CommandBuffer{*this, raw_data[0] },
            CommandBuffer{*this, raw_data[1] }
        );
    }

    [[nodiscard]] std::vector<CommandBuffer> CommandPool::AllocateCommands(uint8_t count, VkCommandBufferLevel buffer_level) {
        allocatedBuffers += count;

        std::vector<VkCommandBuffer> raw_data(count);
        VkCommandBufferAllocateInfo allocInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = commandPool,
            .level = buffer_level,
            .commandBufferCount = count
        };
        EWE_VK(vkAllocateCommandBuffers, logicalDevice, &allocInfo, raw_data.data());
        std::vector<CommandBuffer> ret;
        ret.reserve(count);
        for (auto& cb : raw_data) {
            ret.emplace_back(*this, cb);
        }
        return ret;
    }
    [[nodiscard]] CommandBuffer CommandPool::AllocateCommand(VkCommandBufferLevel buffer_level) {
        allocatedBuffers++;
        VkCommandBuffer raw_data;
        VkCommandBufferAllocateInfo allocInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = commandPool,
            .level = buffer_level,
            .commandBufferCount = 1
        };
        EWE_VK(vkAllocateCommandBuffers, logicalDevice, &allocInfo, &raw_data);
        return CommandBuffer{ *this, raw_data };
    }
}