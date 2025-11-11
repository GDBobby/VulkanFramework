#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/Instance.h"
#include "EightWinds/PhysicalDevice.h"
#include "EightWinds/QueueFamily.h"
#include "EightWinds/Queue.h"

namespace EWE{

    struct LogicalDevice{
        //the physical device is tied to the logical device, it would make sense for the logical device to outright own it
        PhysicalDevice physicalDevice;
        VmaAllocator vmaAllocator;

        //queues are going to be like halfway between the physical and logical device kinda? 
        //the question being, does the queue own a reference to the logical device, or to the physical device?
        //as long as i initialize the physicalDevice first, I can make the queue families const
        const std::vector<QueueFamily> queueFamilies;

        //i think i'll let this get filtered out in the engine
        //i don't think there's a reason to use multiple queues in a single family currently.
        //i might expose it anyways, idk
        std::vector<Queue> queues; 

        VkDevice device;

        operator VkDevice() const { return device; }

        //if a user wanted to customize how the queues are made, id let them pass parameters thru here
        [[nodiscard]] explicit LogicalDevice(
            Instance& instance, 
            VkSurfaceKHR surface, 
            std::function<VkPhysicalDevice(std::vector<VkPhysicalDevice>)> deviceSelector, 
            std::vector<DeviceExtension>& deviceExtensions
        );
    
    };
}