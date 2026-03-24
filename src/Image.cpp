#include "EightWinds/Image.h"

namespace EWE{
    Image::Image(LogicalDevice& _logicalDevice) noexcept
        : logicalDevice{_logicalDevice}
    {

    }
    Image::~Image(){
        //can we assume it was created?
        vmaDestroyImage(logicalDevice.vmaAllocator, image, memory);

        logicalDevice.images.Remove(this);
    }

    bool Image::Create(VmaAllocationCreateInfo const& allocCreateInfo){
        VkImageCreateInfo imgCreateInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .imageType = data.type,
            .format = data.format,
            .extent = data.extent,
            .mipLevels = data.mipLevels,
            .arrayLayers = data.arrayLayers,
            .samples = data.samples,
            .tiling = data.tiling,
            .usage = data.usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,

            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &owningQueue->family.index,
        //use preinitialized if its going to be copied to
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED

        };

        EWE_VK(vmaCreateImage, logicalDevice.vmaAllocator, &imgCreateInfo, &allocCreateInfo, &image, &memory, nullptr);

        logicalDevice.images.Add(this);

        return true;
    }
    bool Image::Create(VmaAllocationCreateInfo const& allocCreateInfo, StagingBuffer* stagedPixelData){
        VkImageCreateInfo imgCreateInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .imageType = data.type,
            .format = data.format,
            .extent = data.extent,
            .mipLevels = data.mipLevels,
            .arrayLayers = data.arrayLayers,
            .samples = data.samples,
            .tiling = data.tiling,
            .usage = data.usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,

        //use preinitialized if its going to be copied to

            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &owningQueue->family.index,
            .initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED
        };
        EWE_ASSERT(false && "not readyy yet");
        return false;
    }

#if EWE_DEBUG_NAMING
    void Image::SetName(std::string_view _name) {
        name = _name;
        logicalDevice.SetObjectName(image, VK_OBJECT_TYPE_IMAGE, name);
    }
#endif
}