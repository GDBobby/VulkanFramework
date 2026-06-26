#include "EightWinds/Backend/DebugMessenger.h"

#include "EightWinds/Reflect/Enum.h"

#if EWE_DEBUG_BOOL
#include "EightWinds/Instance.h"

#include <cstring>

#ifdef __linux
#include <pthread.h>
#endif

namespace EWE {

    std::function<void()> DebugMessenger::validation_callback = nullptr;

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    ) {

        //stealing from github.com/Ak-Elements/Onyx /modules/rhi/private/onyx/rhi/vulkan/debugutilsmessenger
        std::string_view messageTypeString;
        switch (messageType) {
            case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
                messageTypeString = "GENERAL: ";
                break;
            case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
                messageTypeString = "VALIDATION: ";
                break;
            case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
                messageTypeString = "PERFORMANCE: ";
                break;
            default:
                messageTypeString = "UNKNOWN: ";
        }


        switch (messageSeverity) {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
                Log::Normal("VERBOSE-%s%s\n", messageTypeString.data(), pCallbackData->pMessage);
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                Log::Debug("INFO-%s%s\n", messageTypeString.data(), pCallbackData->pMessage);
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: {
                //std::string idName = pCallbackData->pMessageIdName;
                Log::Warning("WARNING-%s%s\n", messageTypeString.data(), pCallbackData->pMessage);
                break;
            }
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: {
                std::string_view msg = pCallbackData->pMessage;
                if (msg.find("THREADING ERROR") != std::string_view::npos) {
            #if defined(__linux__) || defined(__ANDROID__)
                    unsigned long long tid0 = 0, tid1 = 0;
                    bool parsed = false;

                    size_t first = msg.find("thread ");
                    if (first != std::string_view::npos) {
                        size_t second = msg.find("thread ", first + 7);
                        if (second != std::string_view::npos) {
                            int n1 = std::sscanf(msg.data() + first + 7, "%lli", &tid0);
                            int n2 = std::sscanf(msg.data() + second + 7, "%lli", &tid1);
                            parsed = (n1 == 1 && n2 == 1);
                        }
                    }

                    if (parsed) {
                        Log::Error("vulkan [THREADING ERROR] collision between thread %llu and thread %llu\n", tid0, tid1);
                    } 
                    else {
                        Log::Error("couldn't parse thread ids from: %.*s\n", (int)msg.size(), msg.data());
                    }
#else 
#ifdef _WIN32
#endif
#endif
                }
                else{
                    Log::Error("ERROR-%s%s\n", messageTypeString.data(), pCallbackData->pMessage);
                }
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
                Log::Error("validation default: %s\n", pCallbackData->pMessage);
                break;

        }
        //throw std::exception("validition layer \n");
        for(uint32_t i = 0; i < pCallbackData->objectCount; i++){
            auto const& object = pCallbackData->pObjects[i];
            Log::Debug("\t%u - Type[%s] : Handle[%zu] : Name[%s]\n",
                i, Reflect::Enum::ToString(object.objectType).data(), object.objectHandle, object.pObjectName ? object.pObjectName : ""
            );
        }
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