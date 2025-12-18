#pragma once
#include "EightWinds/VulkanHeader.h"


namespace EWE{
#if EWE_DEBUG_BOOL
    struct Instance;

    struct DebugMessenger {
        DebugMessenger(Instance& instance);

        VkDebugUtilsMessengerEXT messenger;
        Instance& instance;

        static std::function<void()> validation_callback;
        
        static bool CheckValidationLayerSupport();
        static VkDebugUtilsMessengerCreateInfoEXT GetPopulatedDebugMessengerCreateInfo();
    };
#endif
}