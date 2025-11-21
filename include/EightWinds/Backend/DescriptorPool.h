#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/LogicalDevice.h"

namespace EWE{
    //might do 3 levels, application life time, long term, and short term. potentially 4 levels, with 1 being per frame
    //i havent made descriptors per frame yet

    //synchronization is per thread
    struct DescriptorPool{

        LogicalDevice& logicalDevice;

        [[nodicard]] explicit DescriptorPool(LogicalDevice& logicalDevice) noexcept
         : logicalDevice{ logicalDevice } {}
        
        VkDescriptorPool pool;

        uint8_t descriptorCount = 0;

        bool operator==(DescriptorPool const& other) const noexcept {
            return pool == other.pool;
        }
    };
}