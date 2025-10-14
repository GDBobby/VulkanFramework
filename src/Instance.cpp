#include "EightWinds/Instance.h"

#include <cstdio>
#include <cassert>
#include <vector>
#include <unordered_set>

namespace EWE{ 
//EWE_DEBUG;
#define enableValidationLayers true

#if enableValidationLayers
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
            logFile << "current frame index - " << VK::Object->frameIndex << std::endl;
#if COMMAND_BUFFER_TRACING
            for (uint8_t i = 0; i < VK::Object->renderCommands.size(); i++) {
                while (VK::Object->renderCommands[i].usageTracking.size() > 0) {
                    for (auto& usage : VK::Object->renderCommands[i].usageTracking.front()) {
                        logFile << "cb" << i << " : " << usage.funcName;
                    }
                    VK::Object->renderCommands[i].usageTracking.pop();
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
        auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(
                instance,
                "vkCreateDebugUtilsMessengerEXT"
            )
        );

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
            EWE_VK(func, instance, debugMessenger, pAllocator);
        }
    }

    VkDebugUtilsMessengerCreateInfoEXT GetPopulatedDebugMessengerCreateInfo(){
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

    void SetupDebugMessenger(VkInstance& instance) {
        if (!enableValidationLayers) { return; }
        VkDebugUtilsMessengerCreateInfoEXT debugUtilCreateInfo = GetPopulatedDebugMessengerCreateInfo();
        CreateDebugUtilsMessengerEXT(instance, &debugUtilCreateInfo, nullptr, &debugMessenger);
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
#endif

bool CheckInstanceExtensions(std::vector<const char*> const& requiredExtensions, std::unordered_map<std::string, bool>& optionalExtensions) {
        uint32_t extensionCount = 0;
        EWE_VK(vkEnumerateInstanceExtensionProperties, nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensions(extensionCount);
        EWE_VK(vkEnumerateInstanceExtensionProperties, nullptr, &extensionCount, extensions.data());

        //std::cout << "available extensions:" << std::endl;
        std::unordered_set<std::string> available;
        for (const auto& extension : extensions) {
            //std::cout << "\t" << extension.extensionName << std::endl;
            available.insert(extension.extensionName);
        }

        //std::cout << "required extensions:" << std::endl;
        for (const auto& required : requiredExtensions) {
            //std::cout << "\t" << required << std::endl;
            if (available.find(required) == available.end()) {
#if GPU_LOGGING
                std::ofstream logFile{ GPU_LOG_FILE, std::ios::app };
                logFile << "Extension is not available! : " << required << std::endl;
                logFile.close();
#endif
                printf("failed to find extension[%s]\n", required);
                assert(false && "Missing required extension");
                return false;
            }
        }
        for(auto& optional : optionalExtensions){
            if(available.find(optional.first.c_str()) != available.end()){
                optional.second = true;
            }
        }
        return true;
    }

    Instance::Instance(const uint32_t api_version, std::vector<const char*> const& requiredExtensions, std::unordered_map<std::string, bool>& optionalExtensions, VkAllocationCallbacks const* allocCallbacks){
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Eight Winds";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 1, 1);
        appInfo.pEngineName = "Eight Winds Engine";
        appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 1, 1);
        appInfo.apiVersion = api_version;
        
        VkInstanceCreateInfo instanceCreateInfo = {};
        instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pApplicationInfo = &appInfo;
        
        std::vector<const char*> all_extensions{};
        all_extensions.reserve(requiredExtensions.size() + optionalExtensions.size());
        std::copy(requiredExtensions.begin(), requiredExtensions.end(), all_extensions.end());
        for(auto& opt : optionalExtensions){
            all_extensions.push_back(opt.first.c_str());
        }

        instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(all_extensions.size());
        instanceCreateInfo.ppEnabledExtensionNames = all_extensions.data();

#if enableValidationLayers
        if (!CheckValidationLayerSupport()) {
            printf("validation layers not available \n");
            assert(false && "validation layers requested, but not available!");
        }
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = GetPopulatedDebugMessengerCreateInfo();
        const char* validationLayers[] = {"VK_LAYER_KHRONOS_validation"};

        instanceCreateInfo.enabledLayerCount = 1;
        instanceCreateInfo.ppEnabledLayerNames = validationLayers;
        instanceCreateInfo.pNext = &debugCreateInfo;
#else
        instanceCreateInfo.enabledLayerCount = 0;
        instanceCreateInfo.pNext = nullptr;
#endif
        EWE_VK(vkCreateInstance, &instanceCreateInfo, allocCallbacks, &instance);
        CheckInstanceExtensions(requiredExtensions, optionalExtensions);

        SetupDebugMessenger(instance);
    }
}