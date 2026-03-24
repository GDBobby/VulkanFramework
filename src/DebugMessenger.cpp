#include "EightWinds/Backend/DebugMessenger.h"

#if EWE_DEBUG_BOOL
#include "EightWinds/Instance.h"

#include <cstring>

namespace EWE {

    std::function<void()> DebugMessenger::validation_callback = nullptr;

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    ) {
        switch (messageSeverity) {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
                Logger::Print<Logger::Debug>("validation verbose: %d : %s\n", messageType, pCallbackData->pMessage);
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                Logger::Print<Logger::Debug>("validation info: %d : %s\n", messageType, pCallbackData->pMessage);
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: {
                //std::string idName = pCallbackData->pMessageIdName;
                Logger::Print<Logger::Warning>("validation warning: %d : %s\n", messageType, pCallbackData->pMessage);
                break;
            }
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: {
                Logger::Print<Logger::Error>("validation error: %d : %s\n", messageType, pCallbackData->pMessage);

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
#if EWE_DEBUG_BOOL
                //print global state here?
                //potentially a callback?
                if (DebugMessenger::validation_callback != nullptr) {
                    DebugMessenger::validation_callback();
                }
#endif
                EWE_Debug_Breakpoint();
                break;
            }
            default:
                Logger::Print<Logger::Error>("validation default: %s \n", pCallbackData->pMessage);
                EWE_UNREACHABLE;
                break;

        }
        //throw std::exception("validition layer \n");
        return VK_FALSE;
    }

    void CreateDebugUtilsMessengerEXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        VkDebugUtilsMessengerEXT* pDebugMessenger
    ) {
        auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

        if (func != nullptr) {
            EWE_VK(func, instance, pCreateInfo, nullptr, pDebugMessenger);
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

    VkDebugUtilsMessengerCreateInfoEXT DebugMessenger::GetPopulatedDebugMessengerCreateInfo() {
        return VkDebugUtilsMessengerCreateInfoEXT {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = debugCallback,
            .pUserData = nullptr  // Optional
        };
    }

    bool DebugMessenger::CheckValidationLayerSupport() {
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

    DebugMessenger::DebugMessenger(Instance& _instance) 
        : instance{_instance} 
    {
        //pass in the createinfo maybe??
        VkDebugUtilsMessengerCreateInfoEXT debugUtilCreateInfo = GetPopulatedDebugMessengerCreateInfo();
        CreateDebugUtilsMessengerEXT(instance.instance, &debugUtilCreateInfo, &messenger);
    }
}
#endif