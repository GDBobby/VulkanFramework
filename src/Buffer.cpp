#include "EightWinds/Buffer.h"

namespace EWE{

    Buffer::~Buffer(){
        if(vmaAlloc != VK_NULL_HANDLE){
            vmaDestroyBuffer(framework.logicalDevice.vmaAllocator, buffer_info.buffer, vmaAlloc);
        }
    }

    Buffer::Buffer(Framework& framework, VkDeviceSize instanceSize, uint32_t instanceCount, VmaAllocationCreateInfo const& vmaAllocCreateInfo, VkBufferUsageFlags2 usageFlags)
        : framework{framework}, 
        usageFlags{ usageFlags } 
        {
            
            //i dont really know how to handle this yet.
            //device specializer holds the properties
            //buit its tempalted, and i don't really want to template this or LogicalDevice
            
        alignmentSize = CalculateAlignment(instanceSize, usageFlags, framework.properties.limits);
        printf("design not finalzied, potentially an error here\n");
        alignmentSize = instanceSize;
        bufferSize = alignmentSize * instanceCount;
        
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.size = bufferSize;
        bufferInfo.usage = usageFlags;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        vmaCreateBuffer(framework.logicalDevice.vmaAllocator, &bufferInfo, &vmaAllocCreateInfo, &buffer_info.buffer, &vmaAlloc, nullptr);
        
        VkBufferDeviceAddressInfo bdaInfo{};
        bdaInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        bdaInfo.pNext = nullptr;
        bdaInfo.buffer = buffer_info.buffer;
        deviceAddress = vkGetBufferDeviceAddress(framework.logicalDevice.device, &bdaInfo);
    }
}