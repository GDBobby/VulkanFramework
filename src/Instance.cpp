#include "EightWinds/Instance.h"

#if EWE_DEBUG_BOOL
#include <cstdio>
#include <cassert>
#endif
#include <vector>
#include <unordered_set>
#include <stdexcept>
#include <iterator>

namespace EWE{ 

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
#if EWE_DEBUG_BOOL
                printf("failed to find extension[%s]\n", required);
#endif
                throw std::runtime_error("missing required extension");
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

    VkInstance CreateInstance(const uint32_t api_version, std::vector<const char*> const& requiredExtensions, std::unordered_map<std::string, bool>& optionalExtensions) {

        VkApplicationInfo appInfo{
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .pApplicationName = "Eight Winds",
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName = "Eight Winds Engine",
            .engineVersion = VK_MAKE_API_VERSION(0, 2, 0, 0),
            .apiVersion = api_version
        };

        std::vector<const char*> all_extensions{};
        all_extensions.reserve(requiredExtensions.size() + optionalExtensions.size());
        std::copy(
            requiredExtensions.begin(),
            requiredExtensions.end(),
            std::back_inserter(all_extensions)
        );
        if (!CheckInstanceExtensions(requiredExtensions, optionalExtensions)) {
            //throw exception probably
            //im not really sure if I want to use exceptions or not
            throw std::runtime_error("failed to get required extensions for instance");
        }
        for (auto& opt : optionalExtensions) {
            if (opt.second) {
                all_extensions.push_back(opt.first.c_str());
            }
        }
        
        VkInstanceCreateInfo instanceCreateInfo{
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = nullptr,
            .pApplicationInfo = &appInfo,
            .enabledLayerCount = 0,
            .enabledExtensionCount = static_cast<uint32_t>(all_extensions.size()),
            .ppEnabledExtensionNames = all_extensions.data()
        };



#if EWE_DEBUG_BOOL
        if (!DebugMessenger::CheckValidationLayerSupport()) {
            printf("validation layers not available \n");
            assert(false && "validation layers requested, but not available!");
        }
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = DebugMessenger::GetPopulatedDebugMessengerCreateInfo();
        const char* validationLayers[] = { "VK_LAYER_KHRONOS_validation" };

        instanceCreateInfo.enabledLayerCount = 1;
        instanceCreateInfo.ppEnabledLayerNames = validationLayers;
        instanceCreateInfo.pNext = &debugCreateInfo;
#endif
        VkInstance instance;
        EWE_VK(vkCreateInstance, &instanceCreateInfo, nullptr, &instance);
        return instance;
    }

    Instance::Instance(const uint32_t api_version, std::vector<const char*> const& requiredExtensions, std::unordered_map<std::string, bool>& optionalExtensions) 
        : api_version{api_version},
        instance{CreateInstance(api_version, requiredExtensions, optionalExtensions)}
#if EWE_DEBUG_BOOL
        ,debugMessenger{*this}
#endif
    {
    }

    Instance::~Instance() {
        vkDestroyInstance(instance, nullptr);
    }
}