#include "EightWinds/PhysicalDevice.h"

#include "EightWinds/Queue.h"

namespace EWE{



    bool IsDeviceSuitable(vkPhysicalDevice device, vkSurfaceKHR surface) {
        bool queuesComplete = FindQueueFamilies(device, surface);

        bool extensionsSupported = CheckDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        vkPhysicalDeviceFeatures supportedFeatures = device.getFeatures();

        return queuesComplete && extensionsSupported && swapChainAdequate &&
            supportedFeatures.samplerAnisotropy;
    }


    PhysicalDevice::PhysicalDevice(Instance& instance, vkSurfaceKHR surface) : instance{instance} {
        
        uint32_t deviceCount = 16;
        std::vector<VkPhysicalDevice> devices(deviceCount);
        EWE_VK(vkEnumeratePhysicalDevices, instance, &deviceCount, devices.data());
        printf("Device count: %d\n", deviceCount);
        
        if (deviceCount == 0) {
            printf("failed to find GPUs with Vulkan support!\n");
            assert(false && "failed to find GPUs with Vulkan support!");
        }
        //bigger score == gooder device
        for (uint32_t i = 0; i < deviceCount; i++) {

            EWE_VK(vkGetPhysicalDeviceProperties, devices[i], &properties);

            uint32_t score = 0;
            
            score += (properties.deviceType == vkPhysicalDeviceType::eDiscreteGpu) * 1000;
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
        
        EWE_VK(vkGetPhysicalDeviceProperties, device, properties);
    }

}