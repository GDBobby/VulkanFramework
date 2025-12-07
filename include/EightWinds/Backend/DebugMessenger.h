#pragma once
#include "EightWinds/VulkanHeader.h"


namespace EWE{
#if EWE_DEBUG_BOOL
    struct Instance;

    struct DebugMessenger {
        DebugMessenger(Instance& instance);

        VkDebugUtilsMessengerEXT messenger;
        Instance& instance;
        
        static bool CheckValidationLayerSupport();
        static VkDebugUtilsMessengerCreateInfoEXT GetPopulatedDebugMessengerCreateInfo();
    };
#endif
}