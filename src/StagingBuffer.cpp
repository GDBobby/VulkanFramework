#include "EightWinds/Backend/StagingBuffer.h"

#include "EightWinds/LogicalDevice.h"
#include "EightWinds/Image.h"

#include <cstring>

namespace EWE{
	StagingBuffer::StagingBuffer(LogicalDevice& _logicalDevice, VkDeviceSize size, const void* data)
        : logicalDevice{_logicalDevice}
    {
        VkBufferCreateInfo bufferCreateInfo{
        	.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
       		.size = size,
        	.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        	.sharingMode = VK_SHARING_MODE_EXCLUSIVE
		};

        VmaAllocationCreateInfo vmaAllocCreateInfo{
        	.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        	.usage = VMA_MEMORY_USAGE_AUTO
		};
        VmaAllocationInfo vmaAllocInfo;
        EWE_VK(vmaCreateBuffer, logicalDevice.vmaAllocator, &bufferCreateInfo, &vmaAllocCreateInfo, &buffer, &vmaAlloc, &vmaAllocInfo);

        bufferSize = size;

        Stage(data, size);
    }
    StagingBuffer::StagingBuffer(LogicalDevice& _logicalDevice, VkDeviceSize size)
        : logicalDevice{ _logicalDevice } 
    {
        VkBufferCreateInfo bufferCreateInfo{
        	.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
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

        EWE_VK(vmaCreateBuffer, logicalDevice.vmaAllocator, &bufferCreateInfo, &vmaAllocCreateInfo, &buffer, &vmaAlloc, &vmaAllocInfo);
    }
	
	
	
    void StagingBuffer::Free() {
        if (buffer == VK_NULL_HANDLE) {
            return;
        }
        vmaDestroyBuffer(logicalDevice.vmaAllocator, buffer, vmaAlloc);
    }
	
    void StagingBuffer::Free() const {
        if (buffer == VK_NULL_HANDLE) {
            return;
        }
        vmaDestroyBuffer(logicalDevice.vmaAllocator, buffer, vmaAlloc);
	}
	
    void StagingBuffer::Stage(const void* data, uint64_t _bufferSize) {
        void* stagingData;
        EWE_ASSERT(_bufferSize <= bufferSize);

        EWE_VK(vmaMapMemory, logicalDevice.vmaAllocator, vmaAlloc, &stagingData);
        memcpy(stagingData, data, bufferSize);
        vmaUnmapMemory(logicalDevice.vmaAllocator, vmaAlloc);
    }
    void StagingBuffer::Map(void*& data) {
        EWE_VK(vmaMapMemory, logicalDevice.vmaAllocator, vmaAlloc, &data);
    }
    void StagingBuffer::Unmap() {
        vmaUnmapMemory(logicalDevice.vmaAllocator, vmaAlloc);
    }
	
}