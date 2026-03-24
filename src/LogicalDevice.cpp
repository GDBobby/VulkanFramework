#include "EightWinds/LogicalDevice.h"

#include <array>

namespace EWE{

    VkDevice LogicalDevice::CreateDevice(VkDeviceCreateInfo& deviceCreateInfo) {
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
        VkDeviceQueueCreateInfo queueCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,// this is just for protected bit, and i think it requires vkdevicequeuecreateinfo2 or something
            .queueCount = 1
        };

        for (uint8_t i = 0; i < physicalDevice.queueFamilies.size(); i++) {
            queueCreateInfo.queueFamilyIndex = physicalDevice.queueFamilies[i].index;
            auto& family = physicalDevice.queueFamilies[i];

            const float familyOffset = (i * 0.01f);

            if (family.SupportsGraphics()) {
                if (family.SupportsSurfacePresent()) {
                    queuePriorities.push_back(1.f - familyOffset);
                }
                else {
                    queuePriorities.push_back(0.85f - familyOffset);
                }
            }
            else if (family.SupportsSurfacePresent()) {
                queuePriorities.push_back(0.7f - familyOffset);
            }
            else if (family.SupportsCompute()) {
                queuePriorities.push_back(0.55f - familyOffset);
            }
            else if (family.SupportsCompute() && family.SupportsTransfer()) {
                queuePriorities.push_back(0.4f - familyOffset);
            }
            else if (family.SupportsTransfer()) {
                queuePriorities.push_back(0.25f - familyOffset);
            }
            else {
                //im not supporting protected bit
                continue;
            }

            if (queuePriorities.size() > queueCreateInfos.size()) {
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
        
        queues.Resize(queueCreateInfos.size());
        for (std::size_t i = 0; i < queueCreateInfos.size(); i++) {
            //VkDevice logicalDeviceExplicit, QueueFamily& family, float priority
            queues.ConstructAt(i, *this, physicalDevice.queueFamilies[queueCreateInfos[i].queueFamilyIndex], queuePriorities[queueCreateInfos[i].queueFamilyIndex]);
        }

        return device;
    }
    
    LogicalDevice::LogicalDevice(
        PhysicalDevice&& physDevice,
        VkDeviceCreateInfo& deviceCreateInfo,
        uint32_t _api_version,
        VmaAllocatorCreateFlags allocatorFlags,
        Backend::FeaturePack const& featurePack,
        Backend::PropertyPack const& propertyPack
    ) noexcept
    : instance{physDevice.instance},
        physicalDevice{std::forward<PhysicalDevice>(physDevice)},
        api_version{_api_version},
        queues{deviceCreateInfo.queueCreateInfoCount},
        device{CreateDevice(deviceCreateInfo)},
        
        bindlessDescriptor{*this},
        
        features{featurePack.features},
        features11{featurePack.features11},
        features12{featurePack.features12},
        features13{featurePack.features13},
        features14{featurePack.features14},
        
        properties{ propertyPack.properties},
        properties11{ propertyPack.properties11},
        properties12{ propertyPack.properties12},
        properties13{ propertyPack.properties13},
        properties14{ propertyPack.properties14},

        garbageDisposal{device}
    {
#if EWE_DEBUG_NAMING
        physicalDevice.name = properties.properties.deviceName;
        BeginLabel = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(vkGetDeviceProcAddr(device, "vkCmdBeginDebugUtilsLabelEXT"));
        EndLabel = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(vkGetDeviceProcAddr(device, "vkCmdEndDebugUtilsLabelEXT"));
        debugUtilsObjectName = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT"));
#endif
        cmdDrawMeshTasks = reinterpret_cast<PFN_vkCmdDrawMeshTasksEXT>(vkGetDeviceProcAddr(device, "vkCmdDrawMeshTasksEXT"));
        cmdDrawMeshTasksIndirect = reinterpret_cast<PFN_vkCmdDrawMeshTasksIndirectEXT>(vkGetDeviceProcAddr(device, "vkCmdDrawMeshTasksIndirectEXT"));
        cmdDrawMeshTasksIndirectCount = reinterpret_cast<PFN_vkCmdDrawMeshTasksIndirectCountEXT>(vkGetDeviceProcAddr(device, "vkCmdDrawMeshTasksIndirectCountEXT"));

        VmaAllocatorCreateInfo allocatorCreateInfo{
            .flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT | VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
            .physicalDevice = physicalDevice.device,
            .device = device,
            .pVulkanFunctions = nullptr,
            .instance = instance.instance,
            .vulkanApiVersion = api_version
        };

        Logger::Print<Logger::Normal>("vma vk version : %zu\n", VMA_VULKAN_VERSION);

        VkResult result = vmaCreateAllocator(&allocatorCreateInfo, &vmaAllocator);
        EWE_VK_RESULT(result);
    }

    LogicalDevice::~LogicalDevice() {
        vkDestroyDevice(device, nullptr);
    }

#if EWE_DEBUG_NAMING
    void LogicalDevice::SetObjectName(void* objectHandle, VkObjectType objectType, std::string_view name) const {
        VkDebugUtilsObjectNameInfoEXT nameInfo{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = objectType,
            .objectHandle = reinterpret_cast<uint64_t>(objectHandle),
            .pObjectName = name.data()
        };
        EWE_VK(debugUtilsObjectName, device, &nameInfo);
    }
#endif
    //a separate function will allow for customizaiton of queues



#if EWE_USING_EXCEPTIONS
    void LogicalDevice::HandleVulkanException(EWEException& renderExcept) {

        //i need to do something here when it's runtime. write to file or something

#if EWE_DEBUG_BOOL
        if (renderExcept.result == VK_ERROR_DEVICE_LOST) {
            Logger::Print<Logger::Error>("device was lost\n");
            PFN_vkGetDeviceFaultInfoEXT GetDeviceFaultInfo = nullptr;
            GetDeviceFaultInfo = reinterpret_cast<PFN_vkGetDeviceFaultInfoEXT>(vkGetDeviceProcAddr(device, "vkGetDeviceFaultInfoEXT"));
            if (GetDeviceFaultInfo == nullptr) {
                throw std::runtime_error("unhandled device lost");
                return;
            }

            VkDeviceFaultCountsEXT faultCounts{
                .sType = VK_STRUCTURE_TYPE_DEVICE_FAULT_COUNTS_EXT,
                .pNext = nullptr,
                .addressInfoCount = 0,
                .vendorInfoCount = 0,
                .vendorBinarySize = 0
            };
            GetDeviceFaultInfo(device, &faultCounts, nullptr);

            VkDeviceFaultInfoEXT faultInfo{
                .sType = VK_STRUCTURE_TYPE_DEVICE_FAULT_INFO_EXT,
                .pAddressInfos = nullptr,
                .pVendorInfos = nullptr,
                .pVendorBinaryData = nullptr
            };

            if (faultCounts.addressInfoCount > 0) {
                faultInfo.pAddressInfos = reinterpret_cast<VkDeviceFaultAddressInfoEXT*>
                    (malloc(sizeof(VkDeviceFaultAddressInfoEXT) * faultCounts.addressInfoCount));
            }
            if (faultCounts.vendorInfoCount > 0) {
                faultInfo.pVendorInfos = reinterpret_cast<VkDeviceFaultVendorInfoEXT*>
                    (malloc(sizeof(VkDeviceFaultVendorInfoEXT) * faultCounts.vendorInfoCount));
            }
            if (faultCounts.vendorBinarySize > 0) {
                faultInfo.pVendorBinaryData = malloc(faultCounts.vendorBinarySize);
            }
            GetDeviceFaultInfo(device, &faultCounts, &faultInfo);

            Logger::Print<Logger::Error>("fault info ~~~\n");
            if (faultInfo.description != nullptr && faultInfo.description[0] != '\0') {
                Logger::Print<Logger::Error>("\tdescription - %s\n", faultInfo.description);
            }
            else {
                Logger::Print<Logger::Error>("\tblank description\n");
            }

            Logger::Print<Logger::Error>("address info - %u\n", faultCounts.addressInfoCount);
            for (uint32_t i = 0; i < faultCounts.addressInfoCount; i++) {
                auto& addrInfo = faultInfo.pAddressInfos[i];
                std::string addrType;
                switch (addrInfo.addressType) {
                    case VK_DEVICE_FAULT_ADDRESS_TYPE_NONE_EXT: addrType = "none"; break;
                    case VK_DEVICE_FAULT_ADDRESS_TYPE_READ_INVALID_EXT: addrType = "invalid read"; break;
                    case VK_DEVICE_FAULT_ADDRESS_TYPE_WRITE_INVALID_EXT: addrType = "invalid write"; break;
                    case VK_DEVICE_FAULT_ADDRESS_TYPE_EXECUTE_INVALID_EXT: addrType = "invalid execute"; break;
                    case VK_DEVICE_FAULT_ADDRESS_TYPE_INSTRUCTION_POINTER_UNKNOWN_EXT: addrType = "unknown instruction"; break;
                    case VK_DEVICE_FAULT_ADDRESS_TYPE_INSTRUCTION_POINTER_INVALID_EXT: addrType = "invalid pointer"; break;
                    case VK_DEVICE_FAULT_ADDRESS_TYPE_INSTRUCTION_POINTER_FAULT_EXT: addrType = "pointer fault"; break;
                    default: addrType = "unknown address type?"; break;
                }
                Logger::Print<Logger::Error>("\taddress info[%u] - [%s]\n\t\t[%zu] - [%zu]\n", i, addrType.c_str(), addrInfo.reportedAddress, addrInfo.addressPrecision);
            }

            Logger::Print<Logger::Error>("\nvendor description - %u\n", faultCounts.vendorInfoCount);
            for (uint32_t i = 0; i < faultCounts.vendorInfoCount; i++) {
                auto& vendorInfo = faultInfo.pVendorInfos[i];
                if (vendorInfo.description[0] != '\0') {
                    Logger::Print<Logger::Error>("[%u]\t %s\n", i, vendorInfo.description);
                }
                Logger::Print<Logger::Error>("\t\tcode[%zu] - data[%zu]\n", vendorInfo.vendorFaultCode, vendorInfo.vendorFaultData);
            }

            Logger::Print<Logger::Error>("vendor binary data address and size - [%zu][%zu]\n", reinterpret_cast<std::size_t>(faultInfo.pVendorBinaryData), faultCounts.vendorBinarySize);
            //once finished with it, probably move this
            free(faultInfo.pAddressInfos);
            free(faultInfo.pVendorInfos);
            free(faultInfo.pVendorBinaryData);
        }
#endif
    }
#endif

} //namespace EWE