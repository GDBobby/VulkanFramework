#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/LogicalDevice.h"

#include <string>

namespace EWE{
    struct Image{
        LogicalDevice& logicalDevice;

        std::string name; //directory, this is the hash key
        uint32_t width;
        uint32_t height;

        uint32_t depth;
        uint32_t arrayLayers;
        uint32_t mipLevels;

        vk::Format format;

        vk::ImageLayout layout;

        //Queue owningQueue; //necessary? need to see the rendergraph first
    };
}