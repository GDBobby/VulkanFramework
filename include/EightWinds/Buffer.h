#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/LogicalDevice.h"


namespace EWE{
    struct StagingBuffer{

    };


    struct Buffer{
        LogicalDevice& logicalDevice;
        //EWE_VK(vmaCreateBuffer, VK::Object->vmaAllocator, &bufferInfo, &vmaAllocCreateInfo, &buffer_info.buffer, &vmaAlloc, nullptr);

        [[nodiscard]] Buffer(VkDeviceSize bufferSize, std::size_t instanceCount, VmaMemoryUsage memoryUsage, vk::BufferUsageFlags);// 
        ~Buffer();

        vk::DescriptorBufferInfo buffer_info;


        vk::DeviceSize bufferSize;
        vk::DeviceSize alignmentSize;
        //write the 
        vk::DeviceSize minOffsetAlignment = 1;

        vk::BufferUsageFlags usageFlags;
        VmaMemoryUsage memoryUsage;

        VmaAllocation vmaAlloc{};

        void* mapped = nullptr;
    };
}