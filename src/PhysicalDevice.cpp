#include "EightWinds/PhysicalDevice.h"

#include <cassert>
#include <algorithm>

namespace EWE{

    std::vector<QueueFamily> EnumerateQueueFamilies(Instance& instance, VkPhysicalDevice device, VkSurfaceKHR surface) {
        uint32_t famPropCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties2(device, &famPropCount, nullptr);
        std::vector<VkQueueFamilyProperties2> vkFamilies(famPropCount);
        for (auto& vkfam : vkFamilies) {
            vkfam.sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
            vkfam.pNext = nullptr;
        }
        vkGetPhysicalDeviceQueueFamilyProperties2(device, &famPropCount, vkFamilies.data());

        std::vector<QueueFamily> queueFamilies{};
        queueFamilies.reserve(vkFamilies.size());
        assert(vkFamilies.size() <= 255); //this would be an embarrassing bug
        for (uint8_t i = 0; i < vkFamilies.size(); i++) {
            VkBool32 tempBool;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &tempBool);
            queueFamilies.emplace_back(i, vkFamilies[i].queueFamilyProperties, tempBool);
        }

        return queueFamilies;
    }
    PhysicalDevice::PhysicalDevice(Instance& instance, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
        : instance{ instance }, device{ physicalDevice }, queueFamilies{ EnumerateQueueFamilies(instance, device, surface) }
    {
    }
    PhysicalDevice::PhysicalDevice(Instance& instance, VkPhysicalDevice physicalDevice, std::vector<QueueFamily> const& queueFamilies)
        : instance{ instance }, device{ physicalDevice }, queueFamilies{ queueFamilies }
    {}

    PhysicalDevice::PhysicalDevice(PhysicalDevice&& moveSrc) noexcept
        : instance{moveSrc.instance},
        device{moveSrc.device},
        queueFamilies{moveSrc.queueFamilies.begin(), moveSrc.queueFamilies.end()}
    {}
}