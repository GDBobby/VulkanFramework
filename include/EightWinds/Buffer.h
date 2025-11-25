#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/LogicalDevice.h"
#include "EightWinds/Framework.h"

#include "vma/include/vk_mem_alloc.h"



namespace EWE{
    struct StagingBuffer{

    };

    template<typename T, typename U>
    constexpr bool BitwiseContains(T const lhs, U const rhs) noexcept{
        return (lhs & rhs) == rhs;
    }

    struct Buffer{
        Framework& framework;

        [[nodiscard]] explicit Buffer(Framework& framework, VkDeviceSize instanceSize, uint32_t instanceCount, VmaAllocationCreateInfo const& vmaAllocCreateInfo, VkBufferUsageFlags2KHR usageFlags);
        ~Buffer();

        VkDescriptorBufferInfo buffer_info;


        VkDeviceSize bufferSize;
        VkDeviceSize alignmentSize;
        //write the 
        VkDeviceSize minOffsetAlignment = 1;

        VkBufferUsageFlags2KHR usageFlags;

        VmaMemoryUsage memoryUsage;
        VmaAllocation vmaAlloc{};

        void* mapped = nullptr;

        VkDeviceAddress deviceAddress;


        [[nodiscard]] static constexpr VkDeviceSize CalculateAlignment(VkDeviceSize instanceSize, VkBufferUsageFlags2KHR usageFlags, VkPhysicalDeviceLimits const& limits) {
            VkDeviceSize minOffsetAlignment = 1;
            
            if(BitwiseContains(usageFlags, VK_BUFFER_USAGE_2_INDEX_BUFFER_BIT) 
            || BitwiseContains(usageFlags, VK_BUFFER_USAGE_2_VERTEX_BUFFER_BIT))
            {
                minOffsetAlignment = 1;
            }
            else if (BitwiseContains(usageFlags, VK_BUFFER_USAGE_2_UNIFORM_BUFFER_BIT)) {
                minOffsetAlignment = limits.minUniformBufferOffsetAlignment;
            }
            else if (BitwiseContains(usageFlags, VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT)) {
                minOffsetAlignment = limits.minStorageBufferOffsetAlignment;
            }
            else if(BitwiseContains(usageFlags, VK_BUFFER_USAGE_2_UNIFORM_TEXEL_BUFFER_BIT)){
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