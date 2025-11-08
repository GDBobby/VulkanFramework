#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/PhysicalDevice.h"
#include "EightWinds/QueueFamily.h"
#include "EightWinds/Queue.h"

namespace EWE{

    //0 extensions is default initialization
    struct DeviceExtension{
        VkBaseInStructure* body;
        const char* name;
        bool required;

        //eventually, when C++26 adds reflection, the name can be deduced.
        DeviceExtension(VkBaseInStructure* body, const char* name, bool required)
            : body{body}, name{name}, required{required} 
        {}

        bool supported = false; //this will be read after construction

        bool FailedRequirements() const {
            return required && !supported;
        }

        bool CheckSupport(std::vector<vkExtensionProperties> const& availableExtensions, PhysicalDevice& physicalDevice) {
            for(auto& avail : availableExtensions){
                if(strcmp(avail.extensionName.data(), name) == 0){
                    supported = true;
                    return true;
                }
            }
            assert(!required);
            supported = false;
            return false;
            /* C++26

            if constexpr (requires { body->sType; }) {
                //skip pnext
                for(auto& member : body){
                    if(std::is_same_v<body->pNext, void*>){
                        continue;
                    }
                    member = member && supported;
                }
            }
            */
        }
    };

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

        vkDevice device;

        operator vkDevice() const { return device; }

        //if a user wanted to customize how the queues are made, id let them pass parameters thru here
        [[nodiscard]] explicit LogicalDevice(Instance& instance, vkSurfaceKHR surface, std::vector<DeviceExtension>& deviceExtensions);
    
    };
}