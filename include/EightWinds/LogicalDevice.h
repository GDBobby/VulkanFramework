#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/PhysicalDevice.h"
#include "EightWinds/Queue.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#endif
#include "vk_mem_alloc.h"

namespace EWE{
    struct LogicalDevice{
        PhysicalDevice& phyiscalDevice;
        VmaAllocator vmaAllocator;

        Queue queues;
    };
}