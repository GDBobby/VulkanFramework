#pragma once

#include "EightWinds/VulkanHeader.h"
#include "LogicalDevice.h"

namespace EWE{
    //might do 3 levels, application life time, long term, and short term. potentially 4 levels, with 1 being per frame
    //i havent made descriptors per frame yet

    //definitely pool per thread
    struct DescriptorPool{

        LogicalDevice& logicalDevice;
        
        vk::DescriptorPool pool;

        uint8_t descriptorCount = 0;
        
    };
}