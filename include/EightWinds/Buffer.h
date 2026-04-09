#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/LogicalDevice.h"

#if EWE_DEBUG_NAMING
#include <stacktrace>
#endif


namespace EWE{

    template<typename T, typename U>
    constexpr bool BitwiseContains(T const lhs, U const rhs) noexcept{
        return (lhs & rhs) == rhs;
    }

    struct Buffer{
        using Barrier = VkBufferMemoryBarrier2;

        LogicalDevice& logicalDevice;
        Queue* owningQueue;
        std::string name;

        [[nodiscard]] Buffer(LogicalDevice& device); //does not get initialized on the GPU
        
        [[nodiscard]] explicit Buffer(
            LogicalDevice& logicalDevice, 
            VkDeviceSize instanceSize, uint32_t instanceCount, 
            VmaAllocationCreateInfo const& vmaAllocCreateInfo, 
            VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
        );
        ~Buffer();

        void Init(VkDeviceSize instanceSize, uint32_t instanceCount,
                  VmaAllocationCreateInfo const& vmaAllocCreateInfo,
                  VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
        );

        Buffer(Buffer const& copySrc) = delete;
        Buffer& operator=(Buffer const& copySrc) = delete;
        Buffer(Buffer&& moveSrc) = delete;
        Buffer& operator=(Buffer&& moveSrc) = delete;

        VkDescriptorBufferInfo buffer_info;
        VkBufferUsageFlags usageFlags;

        VkDeviceSize alignmentSize;
        VkDeviceSize bufferSize;
        //write the 
        VkDeviceSize minOffsetAlignment = 1;


        VmaMemoryUsage memoryUsage;
        VmaAllocation vmaAlloc{};

        bool existsOnTheGPU;
        void* mapped = nullptr;
        void* GetMapped();

        VkDeviceAddress deviceAddress = 0;

        void* Map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        void Unmap() noexcept;
        void Flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        void FlushMin(VkDeviceSize offset);
        void FlushIndex(uint32_t index);

        VkDescriptorBufferInfo DescriptorInfo(VkDeviceSize size, VkDeviceSize offset) const;
        VkDescriptorBufferInfo* DescriptorInfo(VkDeviceSize size, VkDeviceSize offset);

        [[nodiscard]] static VkDeviceSize CalculateAlignment(VkDeviceSize instanceSize, VkBufferUsageFlags usageFlags, VkPhysicalDeviceLimits const& limits); 

    private:
        void CreateTheVkBuffer(VmaAllocationCreateInfo const& vmaAllocCreateInfo);
        void DestroyTheVkBuffer();
    public:

#if EWE_DEBUG_NAMING
        std::stacktrace creation_trace;
#endif
        void SetName(std::string_view name);
    };
}