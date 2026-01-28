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

        //i need to do something here when it's runtime. write to file or something

#if EWE_DEBUG_BOOL
        if (renderExcept.result == VK_ERROR_DEVICE_LOST) {
            printf("ERROR : device was lost\n");
            PFN_vkGetDeviceFaultInfoEXT GetDeviceFaultInfo = nullptr;
            GetDeviceFaultInfo = reinterpret_cast<PFN_vkGetDeviceFaultInfoEXT>(vkGetDeviceProcAddr(logicalDevice.device, "vkGetDeviceFaultInfoEXT"));
            if (GetDeviceFaultInfo == nullptr) {
                throw std::runtime_error("unhandled device lost");
                return;
            }

            VkDeviceFaultCountsEXT faultCounts{};
            faultCounts.sType = VK_STRUCTURE_TYPE_DEVICE_FAULT_COUNTS_EXT;
            faultCounts.pNext = nullptr;
            faultCounts.vendorBinarySize = 0;
            faultCounts.addressInfoCount = 0;
            faultCounts.vendorInfoCount = 0;
            GetDeviceFaultInfo(logicalDevice.device, &faultCounts, nullptr);

            VkDeviceFaultInfoEXT faultInfo{};
            faultInfo.sType = VK_STRUCTURE_TYPE_DEVICE_FAULT_INFO_EXT;
            faultInfo.pAddressInfos = nullptr;
            faultInfo.pVendorInfos = nullptr;
            faultInfo.pVendorBinaryData = nullptr;

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
            GetDeviceFaultInfo(logicalDevice.device, &faultCounts, &faultInfo);

            printf("fault info ~~~\n");
            if (faultInfo.description != nullptr && faultInfo.description[0] != '\0') {
                printf("\tdescription - %s\n", faultInfo.description);
            }
            else {
                printf("\tblank description\n");
            }

            printf("address info - %u\n", faultCounts.addressInfoCount);
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
                printf("\taddress info[%u] - [%s]\n\t\t[%zu] - [%zu]\n", i, addrType.c_str(), addrInfo.reportedAddress, addrInfo.addressPrecision);
            }

            printf("\nvendor description - %u\n", faultCounts.vendorInfoCount);
            for (uint32_t i = 0; i < faultCounts.vendorInfoCount; i++) {
                auto& vendorInfo = faultInfo.pVendorInfos[i];
                if (vendorInfo.description[0] != '\0') {
                    printf("[%u]\t %s\n", i, vendorInfo.description);
                }
                printf("\t\tcode[%zu] - data[%zu]\n", vendorInfo.vendorFaultCode, vendorInfo.vendorFaultData);
            }

            printf("vendor binary data address and size - [%zu][%zu]\n", reinterpret_cast<std::size_t>(faultInfo.pVendorBinaryData), faultCounts.vendorBinarySize);
            //once finished with it, probably move this
            free(faultInfo.pAddressInfos);
            free(faultInfo.pVendorInfos);
            free(faultInfo.pVendorBinaryData);
        }
#endif
    }
#endif
}