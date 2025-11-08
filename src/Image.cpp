#include "EightWinds/Image.h"

namespace EWE{
    void Image::CreateImageWithInfo(const vkImageCreateInfo& imageCreateInfo, VmaAllocationCreateInfo const& vmaAllocCreateInfo) {

        vmaCreateImage(logicalDevice.vmaAllocator, &imageCreateInfo, &vmaAllocCreateInfo, &image, &memory, nullptr);

    }
    void Image::CreateImageWithInfo(const vkImageCreateInfo& imageCreateInfo) {

        VmaAllocationCreateInfo vmaAllocCreateInfo{};
        vmaAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        vmaAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT | VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT;

        CreateImageWithInfo(imageCreateInfo, vmaAllocCreateInfo);

    }
}