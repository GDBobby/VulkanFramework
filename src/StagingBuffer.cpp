#include "EightWinds/Backend/StagingBuffer.h"

#include "EightWinds/Image.h"

namespace EWE{
	StagingBuffer::StagingBuffer(VkDeviceSize size, const void* data) {
        VkBufferCreateInfo bufferCreateInfo{
        	.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
       		.size = size,
        	.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        	.sharingMode = VK_SHARING_MODE_EXCLUSIVE
		};

        VmaAllocationInfo vmaAllocInfo{};
        VmaAllocationCreateInfo vmaAllocCreateInfo{
        	.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        	.usage = VMA_MEMORY_USAGE_AUTO
		};
        EWE_VK(vmaCreateBuffer, VK::Object->vmaAllocator, &bufferCreateInfo, &vmaAllocCreateInfo, &buffer, &vmaAlloc, &vmaAllocInfo);

        bufferSize = size;

        Stage(data, size);
    }
    StagingBuffer::StagingBuffer(VkDeviceSize size) {
        VkBufferCreateInfo bufferCreateInfo{
        	.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        	.size = size,
        	.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        	.sharingMode = VK_SHARING_MODE_EXCLUSIVE
		};

        VmaAllocationInfo vmaAllocInfo{};
        VmaAllocationCreateInfo vmaAllocCreateInfo{
        	.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        	.usage = VMA_MEMORY_USAGE_AUTO
		};
        bufferSize = size;

        EWE_VK(vmaCreateBuffer, VK::Object->vmaAllocator, &bufferCreateInfo, &vmaAllocCreateInfo, &buffer, &vmaAlloc, &vmaAllocInfo);
    }
	
	
	
    void StagingBuffer::Free() {
        if (buffer == VK_NULL_HANDLE) {
            return;
        }
        EWE_VK(vmaDestroyBuffer, VK::Object->vmaAllocator, buffer, vmaAlloc);
    }
	
    void StagingBuffer::Free() const {
        if (buffer == VK_NULL_HANDLE) {
            return;
        }
        EWE_VK(vmaDestroyBuffer, VK::Object->vmaAllocator, buffer, vmaAlloc);
	}
	
    void StagingBuffer::Stage(const void* data, uint64_t bufferSize) {
        void* stagingData;

        EWE_VK(vmaMapMemory, VK::Object->vmaAllocator, vmaAlloc, &stagingData);
        memcpy(stagingData, data, bufferSize);
        EWE_VK(vmaUnmapMemory, VK::Object->vmaAllocator, vmaAlloc);
    }
    void StagingBuffer::Map(void*& data) {
        EWE_VK(vmaMapMemory, VK::Object->vmaAllocator, vmaAlloc, &data);
    }
    void StagingBuffer::Unmap() {
        EWE_VK(vmaUnmapMemory, VK::Object->vmaAllocator, vmaAlloc);
    }
	
	
	void StagingBuffer::StageImage(Image& image, bool mipmapping){
		
	}
	
}