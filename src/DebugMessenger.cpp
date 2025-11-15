#include "EightWinds/Backend/DebugMessenger.h"

#include <cassert>

#define enableValidationLayers EWE_DEBUG_BOOL

#if enableValidationLayers
namespace EWE {

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    ) {

        switch (messageSeverity) {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
                printf("validation verbose: %d : %s\n", messageType, pCallbackData->pMessage);
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                printf("validation info: %d : %s\n", messageType, pCallbackData->pMessage);
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: {
                //std::string idName = pCallbackData->pMessageIdName;
                printf("validation warning: %d : %s\n", messageType, pCallbackData->pMessage);
                break;
            }
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: {
                printf("validation error: %d : %s\n", messageType, pCallbackData->pMessage);

#if GPU_LOGGING
                std::ofstream logFile{ GPU_LOG_FILE, std::ios::app };
                logFile << "current frame index - " << vkObject->frameIndex << std::endl;
#if COMMAND_BUFFER_TRACING
                for (uint8_t i = 0; i < vkObject->renderCommands.size(); i++) {
                    while (vkObject->renderCommands[i].usageTracking.size() > 0) {
                        for (auto& usage : vkObject->renderCommands[i].usageTracking.front()) {
                            logFile << "cb" << i << " : " << usage.funcName;
                        }
                        vkObject->renderCommands[i].usageTracking.pop();
                    }
                }
#endif
                logFile.close();
#endif

                assert(false && "validation layer error");
                break;
            }
            default:
                printf("validation default: %s \n", pCallbackData->pMessage);
                EWE_UNREACHABLE;
                break;

        }
        //throw std::exception("validition layer \n");
        return VK_FALSE;
    }

    void CreateDebugUtilsMessengerEXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pDebugMessenger
    ) {
        auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

        if (func != nullptr) {
            EWE_VK(func, instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        else {
            EWE_VK_RESULT(VK_ERROR_EXTENSION_NOT_PRESENT);
        }
    }

    void DestroyDebugUtilsMessengerEXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks* pAllocator) {
        auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }

    VkDebugUtilsMessengerCreateInfoEXT GetPopulatedDebugMessengerCreateInfo() {
        VkDebugUtilsMessengerCreateInfoEXT debugUtilCreateInfo{};
        debugUtilCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugUtilCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugUtilCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugUtilCreateInfo.pfnUserCallback = debugCallback;
        debugUtilCreateInfo.pUserData = nullptr;  // Optional
        return debugUtilCreateInfo;
    }

    bool CheckValidationLayerSupport() {
        uint32_t layerCount;
        EWE_VK(vkEnumerateInstanceLayerProperties, &layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        EWE_VK(vkEnumerateInstanceLayerProperties, &layerCount, availableLayers.data());

        const char* layerName = "VK_LAYER_KHRONOS_validation";
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        return layerFound;
    }

    DebugMessenger::DebugMessenger(Instance& instance) : instance{instance} {
        if (!enableValidationLayers) { return; }
        VkDebugUtilsMessengerCreateInfoEXT debugUtilCreateInfo = GetPopulatedDebugMessengerCreateInfo();
        //CreateDebugUtilsMessengerEXT(instance, &debugUtilCreateInfo, nullptr, &debugMessenger);
        vkCreateDebugUtilsMessengerEXT(instance.instance, &debugUtilCreateInfo, nullptr, &messenger);
    }
}
#endif