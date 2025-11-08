#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/LogicalDevice.h"

#include "vma/vk_mem_alloc.h"

namespace EWE{
    struct StagingBuffer{

    };

    template<typename T, typename U>
    constexpr bool BitwiseContains(T const lhs, U const rhs) noexcept{
        return (lhs & rhs) == rhs;
    }

    struct Buffer{
        LogicalDevice& logicalDevice;

        Buffer(VkDeviceSize instanceSize, uint32_t instanceCount, VmaAllocationCreateInfo const& vmaAllocCreateInfo, vkBufferUsageFlags2KHR usageFlags)
        ~Buffer();

        vkDescriptorBufferInfo buffer_info;


        vkDeviceSize bufferSize;
        vkDeviceSize alignmentSize;
        //write the 
        vkDeviceSize minOffsetAlignment = 1;

        vkBufferUsageFlags2KHR usageFlags;

        VmaMemoryUsage memoryUsage;
        VmaAllocation vmaAlloc{};

        void* mapped = nullptr;


        static constexpr vkDeviceSize CalculateAlignment(vkDeviceSize instanceSize, vkBufferUsageFlags2KHR usageFlags, vkPhysicalDeviceLimits const& limits) {
            vkDeviceSize minOffsetAlignment = 1;
            
            vkBufferUsageFlagBits::eIndexBuffer;
            if(BitwiseContains(usageFlags, vkBufferUsageFlagBits2KHR::eIndexBuffer) 
            || BitwiseContains(usageFlags, vkBufferUsageFlagBits2KHR::eVertexBuffer))
            {
                minOffsetAlignment = 1;
            }
            else if (BitwiseContains(usageFlags, vkBufferUsageFlagBits2KHR::eUniformBuffer)) {
                minOffsetAlignment = limits.minUniformBufferOffsetAlignment;
            }
            else if (BitwiseContains(usageFlags, vkBufferUsageFlagBits2KHR::eStorageBuffer)) {
                minOffsetAlignment = limits.minStorageBufferOffsetAlignment;
            }
            else if(BitwiseContains(usageFlags, vkBufferUsageFlagBits2KHR::eUniformTexelBuffer)){
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