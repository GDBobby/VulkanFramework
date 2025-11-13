#include "EightWinds/PhysicalDevice.h"

#include "EightWinds/Queue.h"

#include <cassert>
#include <cstdio>

namespace EWE{


    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;

        [[nodiscard]] explicit SwapChainSupportDetails(VkPhysicalDevice device, VkSurfaceKHR surface) noexcept {
            EWE_VK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR, device, surface, &capabilities);
            uint32_t formatCount;
            EWE_VK(vkGetPhysicalDeviceSurfaceFormatsKHR, device, surface, &formatCount, nullptr);

            if (formatCount != 0) {
                formats.resize(formatCount);
                EWE_VK(vkGetPhysicalDeviceSurfaceFormatsKHR, device, surface, &formatCount, formats.data());
            }

            uint32_t presentModeCount;
            EWE_VK(vkGetPhysicalDeviceSurfacePresentModesKHR, device, surface, &presentModeCount, nullptr);

            if (presentModeCount != 0) {
                presentModes.resize(presentModeCount);
                EWE_VK(vkGetPhysicalDeviceSurfacePresentModesKHR, device, surface, &presentModeCount, presentModes.data());
            }
        }
    };


    bool IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, std::vector<DeviceExtension>& deviceExtensions) {
        SwapChainSupportDetails swapChainSupport(device, surface);
        bool swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        
        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        return CheckExtensions(device, deviceExtensions) && 
            swapChainAdequate &&
            supportedFeatures.samplerAnisotropy;
    }


    PhysicalDevice::PhysicalDevice(Instance& instance, std::function<VkPhysicalDevice(std::vector<VkPhysicalDevice>)> deviceSelector)
    : instance{instance} 
    {
        
        uint32_t deviceCount = 16;
        std::vector<VkPhysicalDevice> devices(deviceCount);
        EWE_VK(vkEnumeratePhysicalDevices, instance, &deviceCount, devices.data());

        device = deviceSelector(devices);
    }

}