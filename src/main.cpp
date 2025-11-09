//example
#include "EightWinds/VulkanHeader.h"
#include "EightWinds/Window.h"
#include "EightWinds/LogicalDevice.h"
#include "EightWinds/SwapChain.h"

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
    //potentially, could make headless applications, but i don't personally have interest in supporting that at the moment
    EWE::Window window{instance, 800, 600, "Example Window"};

    
    std::vector<EWE::DeviceExtension> extensions{
        {.body = extAddr, .name = VK_KHR_SWAPCHAIN_EXTENSION_NAME, .required = true},
        {.body = extAddr, .name = VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME, .required = false},
        {.body = extAddr, .name = VK_EXT_MESH_SHADER_EXTENSION_NAME, .required = false},
    };

    //i need a way to request device features

    EWE::LogicalDevice logicalDevice(instance, window.surface, extensions);

    SwapChain swapchain{};
    //from here, create the render graph


    return 0;
}