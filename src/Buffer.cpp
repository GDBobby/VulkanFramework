#include "EightWinds/Buffer.h"

#include "vma/include/vk_mem_alloc.h"
#include <cassert>

namespace EWE{

    Buffer::~Buffer(){
        if(vmaAlloc != VK_NULL_HANDLE){
            vmaDestroyBuffer(framework.logicalDevice.vmaAllocator, buffer_info.buffer, vmaAlloc);
        }
    }

    Buffer::Buffer(Framework& framework, VkDeviceSize instanceSize, uint32_t instanceCount, VmaAllocationCreateInfo const& vmaAllocCreateInfo, VkBufferUsageFlags usageFlags)
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
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.pNext = nullptr;
        bufferInfo.size = bufferSize;
        bufferInfo.usage = usageFlags;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        assert(bufferSize > 0);

        EWE_VK(vmaCreateBuffer, framework.logicalDevice.vmaAllocator, &bufferInfo, &vmaAllocCreateInfo, &buffer_info.buffer, &vmaAlloc, nullptr);
        
        VkBufferDeviceAddressInfo bdaInfo{};
        bdaInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        bdaInfo.pNext = nullptr;
        bdaInfo.buffer = buffer_info.buffer;
        deviceAddress = vkGetBufferDeviceAddress(framework.logicalDevice.device, &bdaInfo);
    }

    void* Buffer::Map(VkDeviceSize size, VkDeviceSize offset) {
        EWE_VK(vmaMapMemory, framework.logicalDevice.vmaAllocator, vmaAlloc, &mapped);
        assert(mapped != nullptr);
        return mapped;
    }

    void Buffer::Unmap() noexcept {
        assert(mapped);
        vmaUnmapMemory(framework.logicalDevice.vmaAllocator, vmaAlloc);
        mapped = nullptr;
    }
    
    void Buffer::Flush(VkDeviceSize size, VkDeviceSize offset) {
        EWE_VK(vmaFlushAllocation, framework.logicalDevice.vmaAllocator, vmaAlloc, offset, size);
    }
    void Buffer::FlushMin(VkDeviceSize offset){
        VkDeviceSize trueOffset = offset - (offset % minOffsetAlignment);
        if(offset != trueOffset){
            //warning maybe?
        }
        EWE_VK(vmaFlushAllocation, framework.logicalDevice.vmaAllocator, vmaAlloc, trueOffset, minOffsetAlignment);
    }
    void Buffer::FlushIndex(uint32_t index) { 
        Flush(alignmentSize, index * alignmentSize); 
    }

    VkDeviceSize Buffer::CalculateAlignment(VkDeviceSize instanceSize, VkBufferUsageFlags usageFlags, VkPhysicalDeviceLimits const& limits) {
        VkDeviceSize minOffsetAlignment = 1;
        
        if(BitwiseContains(usageFlags, VK_BUFFER_USAGE_INDEX_BUFFER_BIT) 
        || BitwiseContains(usageFlags, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT))
        {
            minOffsetAlignment = 1;
        }
        else if (BitwiseContains(usageFlags, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)) {
            minOffsetAlignment = limits.minUniformBufferOffsetAlignment;
        }
        else if (BitwiseContains(usageFlags, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)) {
            minOffsetAlignment = limits.minStorageBufferOffsetAlignment;
        }
        else if(BitwiseContains(usageFlags, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT)){
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
        VkDescriptorBufferInfo Buffer::DescriptorInfo(VkDeviceSize size, VkDeviceSize offset) const {
        VkDescriptorBufferInfo ret = buffer_info;
        ret.offset = offset;
        ret.range = size;
        return ret;
    }

    VkDescriptorBufferInfo* Buffer::DescriptorInfo(VkDeviceSize size, VkDeviceSize offset) {
        buffer_info.offset = offset;
        buffer_info.range = size;
        return &buffer_info;
    }

#if EWE_DEBUG_NAMING
    void Buffer::SetName(std::string_view name) {
        framework.logicalDevice.SetObjectName(buffer_info.buffer, VK_OBJECT_TYPE_BUFFER, name);
    }
#endif
} //namespace EWE