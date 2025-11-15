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
        PhysicalDevice physicalDevice;
        VmaAllocator vmaAllocator;

        //queues are going to be like halfway between the physical and logical device kinda? 
        //the question being, does the queue own a reference to the logical device, or to the physical device?
        //as long as i initialize the physicalDevice first, I can make the queue families const
        std::vector<QueueFamily> queueFamilies;

        //i think ill let the engine handle filtering the queues
        //i don't think there's a reason to use multiple queues in a single family currently.
        //i'll expose it anyways
        std::vector<Queue> queues; 

        VkDevice device;

        operator VkDevice() const { return device; }

        //if a user wanted to customize how the queues are made, id let them pass parameters thru here
        [[nodiscard]] explicit LogicalDevice(
            PhysicalDevice&& physicalDevice,
            VkDeviceCreateInfo& deviceCreateInfo
        );

        uint64_t GetBufferMinimumAlignment(VkBufferUsageFlags2 usageFlags) const;
    
    };
}