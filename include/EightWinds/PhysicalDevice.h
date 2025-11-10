#pragma once
#include "EightWinds/VulkanHeader.h"
#include "EightWinds/PhysicalDeviceHelpers.h"
#include "EightWinds/Instance.h"

#include <vector>

namespace EWE{
    struct PhysicalDevice{
        Instance& instance;

        VkPhysicalDevice                    device;
        //VkPhysicalDeviceLimits              limits; //properties.limits

        VkPhysicalDeviceProperties properties;

        //VkSurfaceKHR surface, std::vector<DeviceExtension>& deviceExtensions
        [[nodiscard]] explicit PhysicalDevice(Instance& instance, std::function<VkPhysicalDevice(std::vector<VkPhysicalDevice>)> deviceSelector) noexcept;
    };
}