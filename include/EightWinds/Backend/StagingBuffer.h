#pragma once

#include "EightWinds/VulkanHeader.h"


#include <cstdint>

namespace EWE{
	//struct Image;
	//struct Buffer;
    struct LogicalDevice;
	
    struct StagingBuffer{
        LogicalDevice& logicalDevice;

        VkBuffer buffer{ VK_NULL_HANDLE };
        VkDeviceSize bufferSize;
        VmaAllocation vmaAlloc{};
        StagingBuffer(LogicalDevice& logicalDevice, VkDeviceSize size);
        StagingBuffer(LogicalDevice& logicalDevice, VkDeviceSize size, const void* data);
        void Free();
        void Free() const;
        void Stage(const void* data, uint64_t bufferSize);
        void Map(void*& data);
        void Unmap();
		
		
		//void StageImage(Image& image, bool mipmapping);
    };
}