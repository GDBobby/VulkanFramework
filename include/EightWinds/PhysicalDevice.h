#include "EightWinds/VulkanHeader.h"
#include "EightWinds/Instance.h"

namespace EWE{
    struct PhysicalDevice{
        Instance& instance;

        VkPhysicalDevice                    device;
        VkPhysicalDeviceLimits              limits;
        VkPhysicalDeviceProperties          properties;
        VkPhysicalDeviceVulkan11Properties  properties11;
        VkPhysicalDeviceVulkan12Properties  properties12;
        VkPhysicalDeviceVulkan13Properties  properties13;

        [[nodiscard]] explicit PhysicalDevice(Instance& instance);
    };
}