#pragma once
#include "EightWinds/VulkanHeader.h"
#include "EightWinds/Instance.h"
#include "EightWinds/QueueFamily.h"

#include <vector>

namespace EWE{
    struct PhysicalDevice{
        Instance& instance;

        VkPhysicalDevice device;

        const std::vector<QueueFamily> queueFamilies;

        //the device is selected separately
        [[nodiscard]] explicit PhysicalDevice(Instance& instance, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
        [[nodiscard]] explicit PhysicalDevice(Instance& instance, VkPhysicalDevice physicalDevice, std::vector<QueueFamily> const& queueFamilies);

        PhysicalDevice(PhysicalDevice&& moveSrc) noexcept;
    };
}