#include "EightWinds/Image.h"

namespace EWE{
    Image::Image(LogicalDevice& logicalDevice) noexcept
        : logicalDevice{logicalDevice}
    {

    }

    bool Image::Create(VmaAllocationCreateInfo const& allocCreateInfo){
        VkImageCreateInfo imgCreateInfo{};
        imgCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imgCreateInfo.pNext = nullptr;
        imgCreateInfo.flags = 0;
        imgCreateInfo.extent = extent;
        imgCreateInfo.format = format;
        imgCreateInfo.arrayLayers = arrayLayers;
        imgCreateInfo.mipLevels = mipLevels;
        imgCreateInfo.usage = usage;
        imgCreateInfo.tiling = tiling;
        imgCreateInfo.samples = samples;
        imgCreateInfo.imageType = type;
        
        //use preinitialized if its going to be copied to
        imgCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        imgCreateInfo.queueFamilyIndexCount = 1;
        imgCreateInfo.pQueueFamilyIndices = &owningQueue->family.index;
        imgCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        EWE_VK(vmaCreateImage, logicalDevice.vmaAllocator, &imgCreateInfo, &allocCreateInfo, &image, &memory, nullptr);
        return true;
    }
    bool Image::Create(VmaAllocationCreateInfo const& allocCreateInfo, StagingBuffer* stagedPixelData){
        VkImageCreateInfo imgCreateInfo{};
        imgCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imgCreateInfo.pNext = nullptr;
        imgCreateInfo.flags = 0;
        imgCreateInfo.extent = extent;
        imgCreateInfo.format = format;
        imgCreateInfo.arrayLayers = arrayLayers;
        imgCreateInfo.mipLevels = mipLevels;
        imgCreateInfo.usage = usage;
        imgCreateInfo.tiling = tiling;
        imgCreateInfo.samples = samples;
        imgCreateInfo.imageType = type;
        
        //use preinitialized if its going to be copied to
        imgCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;

        imgCreateInfo.queueFamilyIndexCount = 1;
        imgCreateInfo.pQueueFamilyIndices = &owningQueue->family.index;
        imgCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        assert(false && "not readyy yet");
        return false;
    }

#if EWE_DEBUG_NAMING
    void Image::SetName(std::string_view name) {
        debugName = name;
        logicalDevice.SetObjectName(image, VK_OBJECT_TYPE_IMAGE, name);
    }
#endif
}