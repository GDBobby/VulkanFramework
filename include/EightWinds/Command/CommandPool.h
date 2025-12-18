#pragma once

#include "EightWinds/VulkanHeader.h"

#include "EightWinds/LogicalDevice.h"
#include "EightWinds/Queue.h"
#include "EightWinds/PerFlight.h"

namespace EWE{
    struct CommandBuffer;
    //command pools per device, per queue, and per thread

    //its recommend to not use vkCommandPoolCreateFlagBits::eResetCommandBuffer
    //allocate maybe 10 or so command buffers per auxilary pool, reset the whole pool when done
    //^ maybe, i need to set up a way to quickly benchmark both, if even necessary.


    struct CommandPool{
        LogicalDevice& logicalDevice;
        Queue& queue;

        VkCommandPool commandPool;

        uint16_t allocatedBuffers = 0;
        VkCommandPoolCreateFlags flags;

        //main rendering thread should not be auxilary, the rest are
        //auxilary pools can reset individual command buffers, main pool resets all at once
        [[nodiscard]] explicit CommandPool(LogicalDevice& logicalDevice, Queue& queue, VkCommandPoolCreateFlags createFlags);

        [[nodiscard]] PerFlight<CommandBuffer> AllocateCommandsPerFlight(VkCommandBufferLevel buffer_level);
        [[nodiscard]] std::vector<CommandBuffer> AllocateCommands(uint8_t count, VkCommandBufferLevel buffer_level);
        [[nodiscard]] CommandBuffer AllocateCommand(VkCommandBufferLevel buffer_level);

        operator VkCommandPool() const {
            return commandPool;
        }
    };

    /*
    //for now, auxiliary pools will use eTransient and eResetCommandBuffer
    struct MainCommandPool{
        //graphics queue
        LogicalDevice& logicalDevice;
        Queue& queue;
        vkCommandPool commandPool;
        
        CommandBuffer const& GetCmdBuf(){

        }
        
    private:
        uint16_t allocatedCommandBufferCount = 256;
        uint16_t usedCommandBufferCount = 0;



    };


    struct AuxiliaryCommandPool{
        LogicalDevice& logicalDevice;
        Queue& queue;

        vkCommandPool commandPool;

        //how many command buffers can be allocated without reallocating the pool
        uint16_t allocatedCommandBufferCount;

        //main rendering thread should not be auxilary, the rest are
        //auxilary pools can reset individual command buffers, main pool resets all at once
        [[nodiscard]] explicit AuxiliaryCommandPool(LogicalDevice& logicalDevice, Queue& queue);
        //release bits?

    private:
        uint16_t currentCmdBufIndex = 0;
    
    };
    */
}