#include "EightWinds/LogicalDevice.h"

namespace EWE{
    
    LogicalDevice::LogicalDevice(
        Instance& instance, 
        VkSurfaceKHR surface, 
        std::function<VkPhysicalDevice(std::vector<VkPhysicalDevice>)> deviceSelector, 
        std::span<DeviceExtension>& deviceExtensions,
    )
    : physicalDevice{instance, deviceSelector},
        queueFamilies{QueueFamily::Enumerate(physicalDevice, surface)}
    {
        
        //since im only doing 1 queue per family, this can be a vector of floats rather than a vector<vector<float>>
        //i read that priorities only matter when there are multiple queues in a family
        //but its a bit vague, so I'll probably set them across families anyways
        std::vector<float> queuePriorities{};
        queuePriorities.reserve(queueFamilies.size());

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pNext = nullptr;// this is just for protected bit, and i think it requires vkdevicequeuecreateinfo2 or something


        for(uint8_t i = 0; i < queueFamilies.size(); i++){
            queueCreateInfo.queueFamilyIndex = queueFamilies[i].index;
            auto& family = queueFamilies[i];

            const float familyOffset = (i * 0.01f);

            if(family.SupportsGraphics()) {
                if(family.SupportsSurfacePresent()) {
                    queuePriorities.push_back(1.f - familyOffset);
                }
                else{
                    queuePriorities.push_back(0.85f - familyOffset);
                }
            }
            else if(family.SupportsSurfacePresent()){
                queuePriorities.push_back(0.7f - familyOffset);
            }
            else if (family.SupportsCompute()){
                queuePriorities.push_back(0.55f - familyOffset);
            }
            else if (family.SupportsCompute() && family.SupportsTransfer()){
                queuePriorities.push_back(0.4f - familyOffset);
            }
            else if (family.SupportsTransfer()){
                queuePriorities.push_back(0.25f - familyOffset);
            }
            else{
                //im not supporting protected bit
                continue;
            }

            if(queuePriorities.size() > queueCreateInfos.size()){
                //ensure queuePriorities doesnt resize
                queueCreateInfo.pQueuePriorities = &queuePriorities.back();
                queueCreateInfos.push_back(queueCreateInfo);
            }

        }

        
        for (int i = 0; i < queueCreateInfos.size(); i++) {
            //this is expecting a c array (pointer to contiguous data)
            //but im only using one queue, so im giving it a pointer to a single point of data
            queueCreateInfos[i].pQueuePriorities = &queuePriorities[i];
        }
        

        uint32_t propCount;
        vkEnumerateDeviceExtensionProperties(physicalDevice.device, nullptr, &propCount, nullptr);
        std::vector<VkExtensionProperties> extensionProperties{propCount};
        vkEnumerateDeviceExtensionProperties(physicalDevice.device, nullptr, &propCount, extensionProperties.data());
/*
        std::vector<const char*> extensionNames{};
        
        for(auto& ext : deviceExtensions){
            if(ext.CheckSupport(extensionProperties)){
                deviceExts.Add(ext.name);
                if(ext.name != nullptr){
                    extensionNames.push_back(ext.name);
                }
            }
        }
*/

        
        VkDeviceCreateInfo deviceCreateInfo = {};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pNext = pNextChain;

        deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();

        /*
        VUID-VkDeviceCreateInfo-pNext-00373
        If the pNext chain includes a VkPhysicalDeviceFeatures2 structure, then pEnabledFeatures must be NULL
        */
        deviceCreateInfo.pEnabledFeatures = nullptr;
        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensionNames.size());
        deviceCreateInfo.ppEnabledExtensionNames = extensionNames.data();
        EWE_VK(vkCreateDevice, physicalDevice.device, &deviceCreateInfo, nullptr, &device);


    }

    //a separate function will allow for customizaiton of queues
}