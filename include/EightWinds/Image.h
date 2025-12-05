#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/LogicalDevice.h"
#include "EightWinds/Queue.h"

#include <string>

namespace EWE{
    //the framework isnt going to provide a method to load images
    //it can be done externally via stb or the like

    struct StagingBuffer;

    struct Image{
        LogicalDevice& logicalDevice;
        VkImage image;
        [[nodiscard]] explicit Image(LogicalDevice& logicalDevice) noexcept;
        operator VkImage() const {
            return image;
        }
#if EWE_DEBUG_NAMING
        std::string debugName;
        void SetName(std::string_view name);
#endif

        Queue* owningQueue;

        std::string name; //directory, this is the hash key for an unordered_set
        VkExtent3D extent;

        uint32_t arrayLayers;
        uint32_t mipLevels;

        VkFormat format;

        VkImageLayout layout;

		VmaAllocation memory;

        int texture_index = -1;

        VkImageCreateFlags createFlags;//this will specify if it's a cube or not. bunch of other flags, like 2d array. not sure what's necessary
        VkImageUsageFlags usage;
        VkImageType type;
        VkSampleCountFlagBits samples;
        VkImageTiling tiling;
        //Queue owningQueue; //necessary? need to see the rendergraph first

        bool Create(VmaAllocationCreateInfo const& allocCreateInfo);

        //this will handle the transfer behind the scenes
        bool Create(VmaAllocationCreateInfo const& allocCreateInfo, StagingBuffer* pixeldata);
    };

    
    /*
        i could create an abstraction for imagecreateinfo
        everything covered by Image can be left out

        samples are passwide or something, i need to check. might be device wide
        i think the only other member not set is usageFlags.
        the only two uses ive had is compute generated, and file sourced.

        if I cover those with a boolean, i can set queues and usageFlags

    constexpr VkImageCreateInfo GetDefaultImageCreateInfo(Image const& image, VkImageUsageFlags usageFlags) noexcept{
        VkImageCreateInfo ret;
        ret.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        ret.pNext = nullptr;
        ret.flags = image.createFlags;
        ret.samples = VK_SAMPLE_COUNT_1_BIT;
        ret.tiling = VK_IMAGE_TILING_OPTIMAL;
        ret.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; 
        //im not sure if something uses an already defined image layout. 
        //i think if im beginning the image in compute it'll start as general?
        ret.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        ret.usage = usageFlags;

        ret.extent.width = image.width;
        ret.extent.height = image.height;
        ret.extent.depth = image.depth;

        ret.mipLevels = image.mipLevels;
        ret.arrayLayers = image.arrayLayers;

        //ret.queueFamilyIndexCount
        //ret.pQueueFamilyIndices

        return ret;
    }
    */
}