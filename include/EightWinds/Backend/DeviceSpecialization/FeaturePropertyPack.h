#pragma once

namespace EWE{
    namespace Backend{
        struct FeaturePack{
            VkPhysicalDeviceFeatures2 features;
            VkPhysicalDeviceVulkan11Features features11;
            VkPhysicalDeviceVulkan12Features features12;
            VkPhysicalDeviceVulkan13Features features13; 
            VkPhysicalDeviceVulkan14Features features14;
        };
        struct PropertyPack{
            VkPhysicalDeviceProperties2 properties;
            VkPhysicalDeviceVulkan11Properties properties11;
            VkPhysicalDeviceVulkan12Properties properties12;
            VkPhysicalDeviceVulkan13Properties properties13;
            VkPhysicalDeviceVulkan14Properties properties14;
        };
    }
}