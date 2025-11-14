#pragma once

#include "EightWinds/Buffer.h"

namespace EWE{

    Buffer::~Buffer(){
        if(vmaAlloc != VK_NULL_HANDLE){
            vmaDestroyBuffer(logicalDevice.vmaAllocator, buffer_info.buffer, vmaAlloc);
        }
    }

    Buffer::Buffer(LogicalDevice& logicalDevice, VkDeviceSize instanceSize, uint32_t instanceCount, VmaAllocationCreateInfo const& vmaAllocCreateInfo, VkBufferUsageFlags2 usageFlags)
        : logicalDevice{logicalDevice}, usageFlags{ usageFlags } {
            
            //i dont really know how to handle this yet.
            //device specializer holds the properties
            //buit its tempalted, and i don't really want to template this or LogicalDevice
            
        //alignmentSize = CalculateAlignment(instanceSize, usageFlags, logicalDevice.physicalDevice.properties.limits);
        printf("design not finalzied, potentially an error here\n");
        alignmentSize = instanceSize;
        bufferSize = alignmentSize * instanceCount;
        
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.size = bufferSize;
        bufferInfo.usage = usageFlags;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        vmaCreateBuffer(logicalDevice.vmaAllocator, &bufferInfo, &vmaAllocCreateInfo, &buffer_info.buffer, &vmaAlloc, nullptr);
    }
}