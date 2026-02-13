#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/Instance.h"
#include "EightWinds/PhysicalDevice.h"
#include "EightWinds/Backend/QueueFamily.h"
#include "EightWinds/Queue.h"
#include "EightWinds/Backend/DeviceSpecialization/FeaturePropertyPack.h"
#include "EightWinds/Backend/Descriptor/Bindless.h"
#include "EightWinds/Backend/GarbageDisposal.h"
#include "EightWinds/Data/RuntimeArray.h"

namespace EWE{
    struct LogicalDevice{
        Instance& instance;
        PhysicalDevice physicalDevice;
        VmaAllocator vmaAllocator;

        const uint32_t api_version;

        //i think ill let the engine handle filtering the queues
        //i don't think there's a reason to use multiple queues in a single family currently.
        HeapBlock<Queue> queues;

        VkDevice device;

        bool operator==(LogicalDevice const& other) const { return device == other.device; }
        operator VkDevice() const { return device; }

        [[nodiscard]] explicit LogicalDevice(
            PhysicalDevice&& physicalDevice,
            VkDeviceCreateInfo& deviceCreateInfo,
            uint32_t api_version,
            VmaAllocatorCreateFlags allocatorFlags,
            Backend::FeaturePack const& featurePack,
            Backend::PropertyPack const& propertyPack
        ) noexcept;
        ~LogicalDevice();
        LogicalDevice(LogicalDevice const& copySrc) = delete;
        LogicalDevice& operator=(LogicalDevice const& copySrc) = delete;
        LogicalDevice(LogicalDevice&& moveSrc) = delete;
        LogicalDevice& operator=(LogicalDevice&& moveSrc) = delete;

        Backend::BindlessDescriptor bindlessDescriptor;

        //i think i can force vulkan 1.4. or at least, i can force earlier API versions to deal with the empty later structs
        const VkPhysicalDeviceFeatures2 features;
        const VkPhysicalDeviceVulkan11Features features11;
        const VkPhysicalDeviceVulkan12Features features12;
        const VkPhysicalDeviceVulkan13Features features13; 
        const VkPhysicalDeviceVulkan14Features features14;

        const VkPhysicalDeviceProperties2 properties;
        const VkPhysicalDeviceVulkan11Properties properties11;
        const VkPhysicalDeviceVulkan12Properties properties12;
        const VkPhysicalDeviceVulkan13Properties properties13;
        const VkPhysicalDeviceVulkan14Properties properties14;

        Backend::GarbageDisposal garbageDisposal;

#if EWE_DEBUG_NAMING
        PFN_vkCmdBeginDebugUtilsLabelEXT BeginLabel;
        PFN_vkCmdEndDebugUtilsLabelEXT EndLabel;

        PFN_vkSetDebugUtilsObjectNameEXT debugUtilsObjectName = nullptr;
        void SetObjectName(void* objectHandle, VkObjectType objectType, std::string_view name) const;
#endif
        PFN_vkCmdDrawMeshTasksEXT cmdDrawMeshTasks;
        PFN_vkCmdDrawMeshTasksIndirectEXT cmdDrawMeshTasksIndirect;
        PFN_vkCmdDrawMeshTasksIndirectCountEXT cmdDrawMeshTasksIndirectCount;

    private:
        //just so i can force construction order. also creates queues
        VkDevice CreateDevice(VkDeviceCreateInfo& deviceCreateInfo);
    };
}