#pragma once

#include "EightWinds/VulkanHeader.h"

namespace EWE{
	struct Image;
	struct Buffer;
	
    struct StagingBuffer{
        VkBuffer buffer{ VK_NULL_HANDLE };
        VkDeviceSize bufferSize;
        VmaAllocation vmaAlloc{};
        StagingBuffer(VkDeviceSize size);
        StagingBuffer(VkDeviceSize size, const void* data);
        void Free();
        void Free() const;
        void Stage(const void* data, uint64_t bufferSize);
        void Map(void*& data);
        void Unmap();
		
		
		void StageImage(Image& image, bool mipmapping);
    };
}