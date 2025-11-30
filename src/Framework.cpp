#include "EightWinds/Framework.h"

namespace EWE{
    
    Framework::Framework(LogicalDevice& logicalDevice) noexcept
    : logicalDevice{logicalDevice},
        shaderFactory{logicalDevice},
        dslCache{logicalDevice.device}
    {
    }

#if EWE_USING_EXCEPTIONS
    void Framework::HandleVulkanException(EWEException& renderExcept) {


        if (renderExcept.result == VK_ERROR_DEVICE_LOST) {
            PFN_vkGetDeviceFaultInfoEXT GetDeviceFaultInfo = nullptr;
            GetDeviceFaultInfo = reinterpret_cast<PFN_vkGetDeviceFaultInfoEXT>(vkGetDeviceProcAddr(logicalDevice.device, "vkGetDeviceFaultInfoEXT"));
            if (GetDeviceFaultInfo == nullptr) {
                //rethrow maybe
                return;
            }

            VkDeviceFaultCountsEXT faultCounts{};
            faultCounts.sType = VK_STRUCTURE_TYPE_DEVICE_FAULT_COUNTS_EXT;
            faultCounts.pNext = nullptr;
            faultCounts.vendorBinarySize = 0;
            faultCounts.addressInfoCount = 0;
            faultCounts.vendorInfoCount = 0;
            GetDeviceFaultInfo(logicalDevice.device, &faultCounts, nullptr);

// Allocate output arrays and query fault data
            VkDeviceFaultInfoEXT faultInfo{};
            faultInfo.sType = VK_STRUCTURE_TYPE_DEVICE_FAULT_INFO_EXT;
            faultInfo.pAddressInfos = reinterpret_cast<VkDeviceFaultAddressInfoEXT*>
                        (malloc(sizeof(VkDeviceFaultAddressInfoEXT) * faultCounts.addressInfoCount));
            faultInfo.pVendorInfos = reinterpret_cast<VkDeviceFaultVendorInfoEXT*>
                        (malloc(sizeof(VkDeviceFaultVendorInfoEXT) * faultCounts.vendorInfoCount));
            faultInfo.pVendorBinaryData = malloc(faultCounts.vendorBinarySize);

            GetDeviceFaultInfo(logicalDevice.device, &faultCounts, &faultInfo);

            //for (uint8_t i = 0; i < VK_MAX_DESCRIPTION_SIZE; i++) {
                printf("fault description - %s\n", faultInfo.description);
            //}


            //once finished with it, probably move this
            free(faultInfo.pAddressInfos);
            free(faultInfo.pVendorInfos);
            free(faultInfo.pVendorBinaryData);
        }
    }
#endif
}