#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/LogicalDevice.h"
#include "EightWinds/Queue.h"

#include <string>

namespace EWE{
    //the framework isnt going to provide a method to load images
    //it can be done externally via stb or the like

    struct Image{
        LogicalDevice& logicalDevice;
        Queue* owningQueue;

        std::string name; //directory, this is the hash key for an unordered_set
        uint32_t width;
        uint32_t height;

        uint32_t depth;
        uint32_t arrayLayers;
        uint32_t mipLevels;

        vkFormat format;

        vkImageLayout layout;

        vkImage image;

		VmaAllocation memory;
        //Queue owningQueue; //necessary? need to see the rendergraph first


        void CreateImageWithInfo(vkImageCreateInfo const& imageCreateInfo, VmaAllocationCreateInfo const& allocCreateInfo);
		void CreateImageWithInfo(vkImageCreateInfo const& imageCreateInfo);
    };

    constexpr GetDefaultImageCreateInfo() noexcept;
}