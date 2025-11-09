#include "EightWinds/Instance.h"

#include <cstdio>
#include <cassert>
#include <vector>
#include <unordered_set>
#include <stdexcept>

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

    Instance::Instance(const uint32_t api_version, std::vector<const char*> const& requiredExtensions, std::unordered_map<std::string, bool>& optionalExtensions) {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pNext = nullptr;
        appInfo.pApplicationName = "Eight Winds";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Eight Winds Engine";
        appInfo.engineVersion = VK_MAKE_API_VERSION(0, 2, 0, 0);
        appInfo.apiVersion = api_version;
        
        VkInstanceCreateInfo instanceCreateInfo = {};
        instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pApplicationInfo = &appInfo;
        
        std::vector<const char*> all_extensions{};
        all_extensions.reserve(requiredExtensions.size() + optionalExtensions.size());
        std::copy(requiredExtensions.begin(), requiredExtensions.end(), all_extensions.end());
        if(!CheckInstanceExtensions(requiredExtensions, optionalExtensions)){
            //throw exception probably
            //im not really sure if I want to use exceptions or not
            throw std::runtime_error("failed to get required extensions for instance");
        }
        for(auto& opt : optionalExtensions){
            if(opt.second){
                all_extensions.push_back(opt.first.c_str());
            }
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
        EWE_VK(vkCreateInstance, &instanceCreateInfo, nullptr, &instance);
    }
}