#pragma once

#include "EightWinds/Buffer.h"

namespace EWE{
    Buffer::Buffer(){
        
    }

    Buffer::~Buffer(){
        if(vmaAlloc != VK_NULL_HANDLE){
            vmaDestroyBuffer(logicalDevice.vmaAllocator, buffer_info.buffer, vmaAlloc);
        }
    }

    Buffer::Buffer(VkDeviceSize instanceSize, uint32_t instanceCount, VmaAllocationCreateInfo const& vmaAllocCreateInfo, vk::BufferUsageFlags2KHR usageFlags)
        : usageFlags{ usageFlags } {

        alignmentSize = CalculateAlignment(instanceSize, usageFlags);
        bufferSize = alignmentSize * instanceCount;
        
        vk::BufferCreateInfo bufferInfo{};
        bufferInfo.size = bufferSize;
        bufferInfo.usage = usageFlags;
        bufferInfo.sharingMode = vk::SharingMode::eExclusive;

        vmaCreateBuffer(logicalDevice.vmaAllocator, &bufferInfo, &vmaAllocCreateInfo, &buffer_info.buffer, &vmaAlloc, nullptr);

        /*
        VmaAllocationCreateInfo vmaAllocCreateInfo{};
        vmaAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        vmaAllocCreateInfo.flags = VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT;
        switch (memoryPropertyFlags) {
        case VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT: {
            vmaAllocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
            break;
        }
        case VK_MEMORY_PROPERTY_HOST_CACHED_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT: {
            vmaAllocCreateInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                VMA_ALLOCATION_CREATE_MAPPED_BIT;
            break;
        }
        case VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT: {
            vmaAllocCreateInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                VMA_ALLOCATION_CREATE_MAPPED_BIT;
            break;
        }
        case VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT: {
            vmaAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
            break;
        }
        case VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT: {
            vmaAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
            break;
        }
        }
        */
    }
}