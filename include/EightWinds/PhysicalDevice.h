#include "EightWinds/VulkanHeader.h"
#include "EightWinds/Instance.h"

namespace EWE{
    struct PhysicalDevice{
        Instance& instance;

        vk::PhysicalDevice                    device;
        vk::PhysicalDeviceLimits              limits;
        vk::PhysicalDeviceProperties          properties;
        vk::PhysicalDeviceVulkan11Properties  properties11;
        vk::PhysicalDeviceVulkan12Properties  properties12;
        vk::PhysicalDeviceVulkan13Properties  properties13;

        [[nodiscard]] explicit PhysicalDevice(Instance& instance, vk::SurfaceKHR surface);
    };
}