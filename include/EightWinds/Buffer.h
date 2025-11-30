#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/LogicalDevice.h"
#include "EightWinds/Framework.h"




namespace EWE{
    struct StagingBuffer{

    };

    template<typename T, typename U>
    constexpr bool BitwiseContains(T const lhs, U const rhs) noexcept{
        return (lhs & rhs) == rhs;
    }

    struct Buffer{
        Framework& framework;

        [[nodiscard]] explicit Buffer(Framework& framework, VkDeviceSize instanceSize, uint32_t instanceCount, VmaAllocationCreateInfo const& vmaAllocCreateInfo, VkBufferUsageFlags usageFlags);
        ~Buffer();

        VkDescriptorBufferInfo buffer_info;


        VkDeviceSize bufferSize;
        VkDeviceSize alignmentSize;
        //write the 
        VkDeviceSize minOffsetAlignment = 1;

        VkBufferUsageFlags usageFlags;

        VmaMemoryUsage memoryUsage;
        VmaAllocation vmaAlloc{};

        void* mapped = nullptr;

        VkDeviceAddress deviceAddress;

        void* Map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        void Unmap() noexcept;
        void Flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        void FlushMin(VkDeviceSize offset);
        void FlushIndex(uint32_t index);

        VkDescriptorBufferInfo DescriptorInfo(VkDeviceSize size, VkDeviceSize offset) const;
        VkDescriptorBufferInfo* DescriptorInfo(VkDeviceSize size, VkDeviceSize offset);

        [[nodiscard]] static VkDeviceSize CalculateAlignment(VkDeviceSize instanceSize, VkBufferUsageFlags usageFlags, VkPhysicalDeviceLimits const& limits); 
    
#if DEBUG_NAMING
        void SetName(std::string_view name) const;
#endif
    };
}