#pragma once
#include "EightWinds/VulkanHeader.h"
#include "EightWinds/Instance.h"

#include <vector>

namespace EWE{
    struct PhysicalDevice{
        Instance& instance;

        VkPhysicalDevice                    device;

        //the device is selected and created separetely. pass in a constructed vkphysicaldevice

        [[nodiscard]] explicit PhysicalDevice(Instance& instance, std::function<VkPhysicalDevice(std::vector<VkPhysicalDevice>)> deviceSelector);
    };
}