#pragma once
#include "EightWinds/VulkanHeader.h"
#include "EightWinds/Instance.h"

namespace EWE{
    struct PhysicalDevice{
        Instance& instance;

        vkPhysicalDevice                    device;
        vkPhysicalDeviceLimits              limits;
        vkPhysicalDeviceProperties          properties;
        vkPhysicalDeviceVulkan11Properties  properties11;
        vkPhysicalDeviceVulkan12Properties  properties12;
        vkPhysicalDeviceVulkan13Properties  properties13;

        [[nodiscard]] explicit PhysicalDevice(Instance& instance, vkSurfaceKHR surface);
    };
}