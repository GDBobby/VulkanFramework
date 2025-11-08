#pragma once
#include "EightWinds/VulkanHeader.h"
#include "EightWinds/Instance.h"

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

        bool CheckSupport(std::vector<VkExtensionProperties> const& availableExtensions);
    };


    struct PhysicalDevice{
        Instance& instance;

        VkPhysicalDevice                    device;
        VkPhysicalDeviceLimits              limits;
        VkPhysicalDeviceProperties          properties;
        VkPhysicalDeviceVulkan11Properties  properties11;
        VkPhysicalDeviceVulkan12Properties  properties12;
        VkPhysicalDeviceVulkan13Properties  properties13;

        [[nodiscard]] explicit PhysicalDevice(Instance& instance, VkSurfaceKHR surface, std::vector<DeviceExtension>& deviceExtensions) noexcept;
    };
}