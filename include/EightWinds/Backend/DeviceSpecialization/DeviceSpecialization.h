#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/LogicalDevice.h"

//one of the main reasons im avoiding hpp in the rest of the app is to reduce compile time. 
//i dont think it's avoidable here without retyping a very large portion of it
//just using structs.hpp would be a nice reduction in compile time but i believe that file was designed to have vulkan.hpp included first???

//anyways, if a user wanted to reduce compile time there's no reason to include this in the full project
//include this file directly from a single cpp, and no where else.
//use extern or whatever function pointers to get DeviceSpecializer functions
//but by doing that, you'll lose almost all the benefits of having this templated in the first place unfortunately

#include <type_traits>
#include <vector>
#include <string_view>
#include <cassert> //remove after initial debug
#include <algorithm>
#include <array>

namespace EWE{

    struct DeviceEvaluation {
        VkPhysicalDevice device = VK_NULL_HANDLE;
#if EWE_DEBUG_BOOL
        std::string name;
        uint32_t api_version;
#endif
        uint64_t score = 0;
        bool passedRequirements = true;
        std::vector<bool> supported_extensions{};

        /*

        * when reflection eventually comes,
        * i'd like to give a detailed report on
        * how properties and limits fail requirements

        * for now, i think only extensions are feasible/reasonable
        */

        std::vector<std::string> failureReport{};

    };
            
    template<uint32_t Vk_Version, typename ExtensionMan, typename FeatureMan, typename PropertyMan>
    struct DeviceSpecializer{

        static constexpr uint32_t vk_version = RoundDownVkVersion(Vk_Version);

        static consteval std::size_t GetExtensionIndex(std::string_view name) {
            return ExtensionMan::NameToIndex(name);
        }

        //i dont think this will be compile evaluated, it'll have to run the whole for loop
        //bool GetExtensionSupport(std::string_view name) const {
        //    return extension_support[ExtensionMan::NameToIndex(name)];
        //}
        //

        //intended to be used with GetExtensionIndex
        bool GetExtensionSupport(std::size_t index) const {
            return extension_support[index];
        }


        std::array<bool, ExtensionMan::Ext_Count()> extension_support;
        FeatureMan features;
        PropertyMan properties;

        template<typename T>
        requires (FeatureMan::template Contains_Type<T>)
        [[nodiscard]] T& GetFeature()        
        {
            if constexpr(std::is_same_v<T, VkPhysicalDeviceFeatures2>){
                return features.base;
            }
            else{
                return features.features.template Get<T>();
            }
        }
        template<typename T>
        requires (FeatureMan::template Contains_Type<T>)
        [[nodiscard]] const T& GetFeature() const
        {
            if constexpr (std::is_same_v<T, VkPhysicalDeviceFeatures2>) {
                return features.base;
            } else {
                return features.features.template Get<T>();
            }
        }

        template<typename T>
        requires (PropertyMan::template Contains_Type<T>)
        [[nodiscard]] T& GetProperty()
        {   
            if constexpr(std::is_same_v<T, VkPhysicalDeviceProperties2>){
                return properties.base;
            }
            else{
                return properties.properties.template Get<T>();
            }   
        }
        template<typename T>
        requires (PropertyMan::template Contains_Type<T>)
        [[nodiscard]] const T& GetProperty() const
        {
            if constexpr (std::is_same_v<T, VkPhysicalDeviceProperties2>) {
                return properties.base;
            } else {
                return properties.properties.template Get<T>();
            }
        }

        [[nodiscard]] std::vector<DeviceEvaluation> ScorePhysicalDevices(VkInstance instance) {
            //im not handling requried properties/limits yet. might just wait until reflection for that.

            uint32_t deviceCount;
            EWE_VK(vkEnumeratePhysicalDevices, instance, &deviceCount, nullptr);
            std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
            EWE_VK(vkEnumeratePhysicalDevices, instance, &deviceCount, physicalDevices.data());

            std::vector<DeviceEvaluation> ret{};
            ret.reserve(deviceCount);

            int16_t chosenDevice = -1;
            uint64_t highest_score = 0;

            for (auto& dev : physicalDevices) {
                auto filtered_features = features.Populate(dev);
                properties.Populate(dev);

                auto& devEval = ret.emplace_back();
                devEval.device = dev;
#if EWE_DEBUG_BOOL
                devEval.name = GetProperty<VkPhysicalDeviceProperties2>().properties.deviceName;
                devEval.api_version = GetProperty<VkPhysicalDeviceProperties2>().properties.apiVersion;
#endif

                auto featureScore = filtered_features.Score(dev);
                devEval.passedRequirements = devEval.passedRequirements && featureScore.metRequirements;
                devEval.score += featureScore.score;
                auto propertyScore = properties.Score(dev);
                devEval.passedRequirements = devEval.passedRequirements && propertyScore.metRequirements;
                devEval.score += propertyScore.score;

                uint32_t propCount;
                EWE_VK(vkEnumerateDeviceExtensionProperties, dev, nullptr, &propCount, nullptr);
                std::vector<VkExtensionProperties> extensionProperties{ propCount };
                EWE_VK(vkEnumerateDeviceExtensionProperties, dev, nullptr, &propCount, extensionProperties.data());

                devEval.supported_extensions = ExtensionMan::GetSupport(extensionProperties);
                assert(devEval.supported_extensions.size() == extension_support.size());
                //^internal dev check, remove the cassert

                for (uint16_t i = 0; i < extension_support.size(); i++) {
                    if (devEval.supported_extensions[i]) {
                        devEval.score += ExtensionMan::scores[i];
                    }
                    else if(ExtensionMan::required[i]){
                        devEval.passedRequirements = false;
                        devEval.failureReport.push_back(ExtensionMan::names[i].data());
                    }
                }


            }
            std::ranges::sort(ret,
                              [](const DeviceEvaluation& a, const DeviceEvaluation& b)
                              {
                                  if (a.passedRequirements != b.passedRequirements)
                                      return a.passedRequirements > b.passedRequirements;
                                  return a.score > b.score;
                              }
            );

            return ret;
        }

        [[nodiscard]] LogicalDevice ConstructDevice(
            DeviceEvaluation& deviceEval, 
            PhysicalDevice&& physicalDevice, 
            VkBaseInStructure* pNextChain,
            uint32_t api_version,
            VmaAllocatorCreateFlags vmaAllocatorFlags
        ){


            features.PopulateInPlace(physicalDevice.device);
            
            //auto featureCopyForScoring = features;
            //featureCopyForScoring.Populate(physicalDevice.device);
            properties.Populate(physicalDevice.device);
            //check extensions
            
            VkDeviceCreateInfo deviceCreateInfo{};
            deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            deviceCreateInfo.pNext = pNextChain;
            
            //extension_support = deviceEval.supported_extensions;
            assert(deviceEval.supported_extensions.size() == extension_support.size());
            for (uint16_t i = 0; i < deviceEval.supported_extensions.size(); i++) {
            //theres a smarter copy for this i just dont feel like loking for it rn
                extension_support[i] = deviceEval.supported_extensions[i];
            }

            deviceCreateInfo.ppEnabledLayerNames = nullptr;
            deviceCreateInfo.enabledLayerCount = 0;

            //deviceCreateInfo.queueCreateInfoCount
            //^handled inside the LogicalDevice constructor

            std::vector<const char*> active_extensions{};
            for (uint16_t i = 0; i < extension_support.size(); i++) {
                if (extension_support[i]) {
                    active_extensions.push_back(ExtensionMan::names[i].data());
                }
            }
            deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(active_extensions.size());
            deviceCreateInfo.ppEnabledExtensionNames = active_extensions.data();

            return LogicalDevice(std::forward<PhysicalDevice>(physicalDevice), deviceCreateInfo, api_version, vmaAllocatorFlags);
        }

    };
}