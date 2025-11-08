//example
#include "EightWinds/VulkanHeader.h"
#include "EightWinds/Window.h"
#include "EightWinds/LogicalDevice.h"

#include <cstdint>
#include <cstdio>


int main(){
    
    //create an instance


    std::vector<const char*> requiredExtensions{
#ifdef _DEBUG
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,

#endif
    };
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    if(glfwExtensionCount != 0){
        assert(glfwExtensions != nullptr);
    }
    else{
        printf("glfw required 0 extensions, suspicious\n");
    }
    for(uint32_t i = 0; i < glfwExtensionCount; ++i){
        requiredExtensions.push_back(glfwExtensions[i]);
    }

    std::unordered_map<std::string, bool> optionalExtensions{};

    /*
        Instance(
            const uint32_t api_version, 
            std::vector<const char*> const& requiredExtensions, 
            std::unordered_map<std::string, bool>& optionalExtensions, 
            vkAllocationCallbacks const* allocCallbacks
        );
    */
    EWE::Instance instance(VK_MAKE_API_VERSION(0, 1, 4, 0), requiredExtensions, optionalExtensions, nullptr);

    //once the instance is created, create a surface
    //the surface needs to be known, to check if the physical devices can render to it
    //potentially, could make headless applications, but i don't personally have interest in supporting that

    EWE::Window window{instance, 800, 600, "Example Window"};

    EWE::LogicalDevice logicalDevice(instance, window.surface);

    //from here, create the render graph


    return 0;
}