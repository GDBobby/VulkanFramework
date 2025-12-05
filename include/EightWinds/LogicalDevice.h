#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/Instance.h"
#include "EightWinds/PhysicalDevice.h"
#include "EightWinds/Backend/QueueFamily.h"
#include "EightWinds/Queue.h"

namespace EWE{

    //the physical device can technically be used by more than 1 logical device
    //i dont believe there's would be a benefit to that, besides offloading some synchronization to the OS/driver/whatever
    //with a correct rendergraph, that shouldn't be an issue

    //so following that logic, the logicaldevice will own the physicaldevice. 

    struct LogicalDevice{
        Instance& instance;
        PhysicalDevice physicalDevice;
        VmaAllocator vmaAllocator;

        const uint32_t api_version;

        //i think ill let the engine handle filtering the queues
        //i don't think there's a reason to use multiple queues in a single family currently.
        std::vector<Queue> queues; 

        VkDevice device;

        operator VkDevice() const { return device; }

        [[nodiscard]] explicit LogicalDevice(
            PhysicalDevice&& physicalDevice,
            VkDeviceCreateInfo& deviceCreateInfo,
            uint32_t api_version,
            VmaAllocatorCreateFlags allocatorFlags
        ) noexcept;

        //uint64_t GetBufferMinimumAlignment(VkBufferUsageFlags2 usageFlags) const;

#if EWE_DEBUG_NAMING
        PFN_vkCmdBeginDebugUtilsLabelEXT BeginLabel;
        PFN_vkCmdEndDebugUtilsLabelEXT EndLabel;

        PFN_vkSetDebugUtilsObjectNameEXT debugUtilsObjectName = nullptr;
        void SetObjectName(void* objectHandle, VkObjectType objectType, std::string_view name) const;
#endif
    };
}