#include "EightWinds/VulkanHeader.h"
namespace EWE{
    struct Queue{
        VkQueueFamilyProperties properties;
        VkQueueFamilyProperties2 properties2;
        VkQueue queue;
    };
}

//vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR