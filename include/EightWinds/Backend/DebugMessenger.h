#pragma once
#include "EightWinds/VulkanHeader.h"

#define Enable_Validation_Layers EWE_DEBUG_BOOL

namespace EWE{
#if Enable_Validation_Layers
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