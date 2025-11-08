#include "EightWinds/PhysicalDevice.h"

#include "EightWinds/Queue.h"

#include <cassert>

namespace EWE{

    bool DeviceExtension::CheckSupport(std::vector<VkExtensionProperties> const& availableExtensions) {
            for(auto& avail : availableExtensions){
                if(strcmp(avail.extensionName, name) == 0){
                    supported = true;
                    return true;
                }
            }
            //this can be used for checking devices, requirement needs to be checked separately
            //assert(!required);
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

    bool CheckExtensions(VkPhysicalDevice device, std::vector<DeviceExtension>& deviceExtensions){
        uint32_t propertyCount;
        EWE_VK(vkEnumerateDeviceExtensionProperties, device, nullptr, &propertyCount, nullptr);
        std::vector<VkExtensionProperties> extensionProperties(propertyCount);
        EWE_VK(vkEnumerateDeviceExtensionProperties, device, nullptr, &propertyCount, extensionProperties.data());

        for(auto& ext : deviceExtensions){
            if(ext.CheckSupport(extensionProperties)){
                if(ext.required){
                    return false;
                }
            }
        }
        return true;
    }

    bool IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, std::vector<DeviceExtension>& deviceExtensions) {
        SwapChainSupportDetails swapChainSupport(device, surface);
        bool swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        
        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        return CheckExtensions(device, deviceExtensions) && 
            swapChainAdequate &&
            supportedFeatures.samplerAnisotropy;
    }


    PhysicalDevice::PhysicalDevice(Instance& instance, VkSurfaceKHR surface, std::vector<DeviceExtension>& deviceExtensions) noexcept
    : instance{instance} 
    {
        
        uint32_t deviceCount = 16;
        std::vector<VkPhysicalDevice> devices(deviceCount);
        EWE_VK(vkEnumeratePhysicalDevices, instance, &deviceCount, devices.data());
        printf("Device count: %d\n", deviceCount);
        
        if (deviceCount == 0) {
            printf("failed to find GPUs with Vulkan support!\n");
            assert(false && "failed to find GPUs with Vulkan support!");
        }
        
        //bigger score == gooder device
        std::list<std::pair<uint32_t, uint32_t>> deviceScores{};
        for (uint32_t i = 0; i < deviceCount; i++) {

            EWE_VK(vkGetPhysicalDeviceProperties, devices[i], &properties);

            uint32_t score = 0;
            
            score += (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) * 1000;
            score += properties.limits.maxImageDimension2D;
            std::string_view deviceNameTemp = properties.deviceName;
#if AMD_TARGET
            if (deviceNameTemp.find("AMD") == deviceNameTemp.npos) {
                score = 0;
            }
#elif NVIDIA_TARGET
            if (deviceNameTemp.find("GeForce") == deviceNameTemp.npos) {
                score = 0;
            }
#elif INTEGRATED_TARGET
            if (vkObject->properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
                score = 0;
            }
#endif

            printf("Device Name:Score %s:%u \n", deviceNameTemp.data(), score);
            for (auto iter = deviceScores.begin(); iter != deviceScores.end(); iter++) {

                //big to little
                if (iter->first < score) {
                    deviceScores.insert(iter, { score, i });
                }
            }
            if (deviceScores.size() == 0) {
                deviceScores.push_back({ score, i });
            }
        }
    
        for (auto iter = deviceScores.begin(); iter != deviceScores.end(); iter++) {
            if (IsDeviceSuitable(devices[iter->second])) {
                device = devices[iter->second];
                break;
            }
            else {
                printf("device unsuitable \n");
            }
        }

        if (device == VK_NULL_HANDLE) {
            printf("failed to find a suitable GPU! \n");
            assert(false && "failed to find a suitable GPU!");
        }
        
        //earlier we were using the sampled devices to check if they supported properties we wanted
        //now, we'll set the sampled properties to the selected device
        EWE_VK(vkGetPhysicalDeviceProperties, device, &properties);
    }

}