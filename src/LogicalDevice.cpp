#include "EightWinds/LogicalDevice.h"

namespace EWE{
    
    LogicalDevice::LogicalDevice(
        PhysicalDevice&& physDevice,
        VkDeviceCreateInfo& deviceCreateInfo,
        uint32_t api_version,
        VmaAllocatorCreateFlags allocatorFlags,
        Backend::FeaturePack const& featurePack,
        Backend::PropertyPack const& propertyPack
    ) noexcept
    : instance{physDevice.instance},
        physicalDevice{std::forward<PhysicalDevice>(physDevice)},
        api_version{api_version},
        
        features{featurePack.features},
        features11{featurePack.features11},
        features12{featurePack.features12},
        features13{featurePack.features13},
        features14{featurePack.features14},
        
        properties{ propertyPack.properties},
        properties11{ propertyPack.properties11},
        properties12{ propertyPack.properties12},
        properties13{ propertyPack.properties13},
        properties14{ propertyPack.properties14}
    {
#if EWE_DEBUG_NAMING
        physicalDevice.name = properties.properties.deviceName;
#endif

        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        //pnext is set before coming into this function
        //extensions are set before coming into this function

        /*
        VUID-VkDeviceCreateInfo-pNext-00373
        If the pNext chain includes a VkPhysicalDeviceFeatures2 structure, then pEnabledFeatures must be NULL
        */
        deviceCreateInfo.pEnabledFeatures = nullptr;

        //queue info will be set, then the device is created

        //since im only doing 1 queue per family, this can be a vector of floats rather than a vector<vector<float>>
        //i read that priorities only matter when there are multiple queues in a family
        //but its a bit vague, so I'll probably set them across families anyways
        std::vector<float> queuePriorities{};
        queuePriorities.reserve(physicalDevice.queueFamilies.size());

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.pNext = nullptr;// this is just for protected bit, and i think it requires vkdevicequeuecreateinfo2 or something
        queueCreateInfo.queueCount = 1;

        for(uint8_t i = 0; i < physicalDevice.queueFamilies.size(); i++){
            queueCreateInfo.queueFamilyIndex = physicalDevice.queueFamilies[i].index;
            auto& family = physicalDevice.queueFamilies[i];

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

        deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();

        EWE_VK(vkCreateDevice, physicalDevice.device, &deviceCreateInfo, nullptr, &device);

        queues.reserve(queueCreateInfos.size());
        for(auto& qci : queueCreateInfos){
            //VkDevice logicalDeviceExplicit, QueueFamily& family, float priority
            queues.emplace_back(*this, physicalDevice.queueFamilies[qci.queueFamilyIndex], queuePriorities[qci.queueFamilyIndex]);
        }

#if EWE_DEBUG_NAMING
        BeginLabel = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(vkGetDeviceProcAddr(device, "vkCmdBeginDebugUtilsLabelEXT"));
        EndLabel = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(vkGetDeviceProcAddr(device, "vkCmdEndDebugUtilsLabelEXT"));
        debugUtilsObjectName = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT"));
#endif


        VmaAllocatorCreateInfo allocatorCreateInfo{};
        allocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT | VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
        allocatorCreateInfo.vulkanApiVersion = api_version;
        allocatorCreateInfo.physicalDevice = physicalDevice.device;
        allocatorCreateInfo.device = device;
        allocatorCreateInfo.instance = instance.instance;
        allocatorCreateInfo.pVulkanFunctions = nullptr;
        VkResult result = vmaCreateAllocator(&allocatorCreateInfo, &vmaAllocator);
        EWE_VK_RESULT(result);
    }

#if EWE_DEBUG_NAMING
    void LogicalDevice::SetObjectName(void* objectHandle, VkObjectType objectType, std::string_view name) const {
        VkDebugUtilsObjectNameInfoEXT nameInfo{};
        nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        nameInfo.pNext = nullptr;
        nameInfo.objectHandle = reinterpret_cast<uint64_t>(objectHandle);
        nameInfo.objectType = objectType;
        nameInfo.pObjectName = name.data();
        EWE_VK(debugUtilsObjectName, device, &nameInfo);
    }
#endif
    //a separate function will allow for customizaiton of queues
}