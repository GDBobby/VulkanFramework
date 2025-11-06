#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/LogicalDevice.h"


namespace EWE{
    struct StagingBuffer{

    };

    template<typename T, typename U>
    constexpr bool BitwiseContains(T const lhs, U const rhs) noexcept{
        return (lhs & rhs) == rhs;
    }

    struct Buffer{
        LogicalDevice& logicalDevice;

    Buffer::Buffer(VkDeviceSize instanceSize, uint32_t instanceCount, VmaAllocationCreateInfo const& vmaAllocCreateInfo, vk::BufferUsageFlags2KHR usageFlags)
        ~Buffer();

        vk::DescriptorBufferInfo buffer_info;


        vk::DeviceSize bufferSize;
        vk::DeviceSize alignmentSize;
        //write the 
        vk::DeviceSize minOffsetAlignment = 1;

        vk::BufferUsageFlags2KHR usageFlags;
        VmaMemoryUsage memoryUsage;

        VmaAllocation vmaAlloc{};

        void* mapped = nullptr;


        static constexpr vk::DeviceSize CalculateAlignment(vk::DeviceSize instanceSize, vk::BufferUsageFlags2KHR usageFlags, vk::PhysicalDeviceLimits const& limits) {
            vk::DeviceSize minOffsetAlignment = 1;
            
            vk::BufferUsageFlagBits::eIndexBuffer;
            if(BitwiseContains(usageFlags, vk::BufferUsageFlagBits2KHR::eIndexBuffer) 
            || BitwiseContains(usageFlags, vk::BufferUsageFlagBits2KHR::eVertexBuffer))
            {
                minOffsetAlignment = 1;
            }
            else if (BitwiseContains(usageFlags, vk::BufferUsageFlagBits2KHR::eUniformBuffer)) {
                minOffsetAlignment = limits.minUniformBufferOffsetAlignment;
            }
            else if (BitwiseContains(usageFlags, vk::BufferUsageFlagBits2KHR::eStorageBuffer)) {
                minOffsetAlignment = limits.minStorageBufferOffsetAlignment;
            }
            else if(BitwiseContains(usageFlags, vk::BufferUsageFlagBits2KHR::eUniformTexelBuffer)){
                //does texel care if its uniform or storage?
                //do i push it into the above?
                minOffsetAlignment = limits.minTexelBufferOffsetAlignment;
            }

            if (minOffsetAlignment > 0) {
                //printf("get alignment size : %zu \n", (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1));
                return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
            }
            return instanceSize;
        }
    };
}