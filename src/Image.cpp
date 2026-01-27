#include "EightWinds/Image.h"

namespace EWE{
    Image::Image(LogicalDevice& logicalDevice) noexcept
        : logicalDevice{logicalDevice}
    {

    }

    bool Image::Create(VmaAllocationCreateInfo const& allocCreateInfo){
        VkImageCreateInfo imgCreateInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .imageType = type,
            .format = format,
            .extent = extent,
            .mipLevels = mipLevels,
            .arrayLayers = arrayLayers,
            .samples = samples,
            .tiling = tiling,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,

            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &owningQueue->family.index,
        //use preinitialized if its going to be copied to
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED

        };

        EWE_VK(vmaCreateImage, logicalDevice.vmaAllocator, &imgCreateInfo, &allocCreateInfo, &image, &memory, nullptr);
        return true;
    }
    bool Image::Create(VmaAllocationCreateInfo const& allocCreateInfo, StagingBuffer* stagedPixelData){
        VkImageCreateInfo imgCreateInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .imageType = type,
            .format = format,
            .extent = extent,
            .mipLevels = mipLevels,
            .arrayLayers = arrayLayers,
            .samples = samples,
            .tiling = tiling,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,

        //use preinitialized if its going to be copied to

            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &owningQueue->family.index,
            .initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED
        };
#if EWE_DEBUG_BOOL
        assert(false && "not readyy yet");
#endif
        return false;
    }

#if EWE_DEBUG_NAMING
    void Image::SetName(std::string_view name) {
        debugName = name;
        logicalDevice.SetObjectName(image, VK_OBJECT_TYPE_IMAGE, name);
    }
#endif
}