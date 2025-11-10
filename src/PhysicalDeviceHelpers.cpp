#include "EightWinds/PhysicalDeviceHelpers.h"

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

    
    std::vector<FeatureManager::FailedFeatureRequirement> FeatureManager::CheckFeatures(VkPhysicalDevice physicalDevice){
        VkBaseInStructure const* inCurrent = reinterpret_cast<VkBaseInStructure*>(incomingFeatures);
        uint64_t totalSize = 0;
        for(auto& size : sizes){
            totalSize += size;
        }
        outFeatures = reinterpret_cast<VkPhysicalDeviceFeatures2*>(malloc(totalSize));
        VkBaseInStructure* outCurrent = reinterpret_cast<VkBaseInStructure*>(outFeatures);
        for(auto& size : sizes){
            memcpy(outCurrent, inCurrent, size);
            outCurrent = reinterpret_cast<VkBaseInStructure*>(reinterpret_cast<std::size_t>(outCurrent) + size);
            inCurrent = inCurrent->pNext;
        }

        vkGetPhysicalDeviceFeatures2(physicalDevice, incomingFeatures);

        return Compare();
    }
    

    std::vector<FeatureManager::FailedFeatureRequirement> FeatureManager::Compare(){
        std::vector<FeatureManager::FailedFeatureRequirement> ret{};


        VkBaseInStructure const* inCurrent = reinterpret_cast<VkBaseInStructure*>(incomingFeatures);
        uint64_t totalSize = 0;
        for(auto& size : sizes){
            totalSize += size;
        }
        VkBaseInStructure* outCurrent = reinterpret_cast<VkBaseInStructure*>(outFeatures);
        for(uint16_t j = 0; j < sizes.size(); j++){
            auto& size = sizes[j];
            //lets do some pseudo reflection
            //this is only going to work because every member is a VkBool32
            //otherwise i would need some more detailed, (hand crafted pre-C++26) reflection
            std::size_t inAddr = reinterpret_cast<std::size_t>(inCurrent);
            std::size_t outAddr = reinterpret_cast<std::size_t>(outCurrent);
            for(std::size_t i = sizeof(VkBaseInStructure); i < size; i += sizeof(VkBool32)){
                VkBool32* inVal = reinterpret_cast<VkBool32*>(inAddr + i);
                VkBool32* outVal = reinterpret_cast<VkBool32*>(outAddr + i);
                if((*inVal) && !(*outVal)){
                    //failed to meet requirement
                    ret.push_back(
                        FeatureManager::FailedFeatureRequirement{
                            .indexInPNextChain = j,
                            .offsetWithinStruct = static_cast<uint16_t>((i - sizeof(VkBaseInStructure)) / sizeof(VkBool32))
                        }
                    );
                }
            }

            outCurrent = reinterpret_cast<VkBaseInStructure*>(reinterpret_cast<std::size_t>(outCurrent) + size);
            inCurrent = inCurrent->pNext;
        }
    }

}