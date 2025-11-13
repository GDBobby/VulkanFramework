//example
#include "EightWinds/VulkanHeader.h"
#include "EightWinds/Window.h"
#include "EightWinds/LogicalDevice.h"
#include "EightWinds/Swapchain.h"

#include "EightWinds/Helpers/DeviceExtensionHandling.h"

#include <cstdint>
#include <cstdio>
#include <cassert>


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
    EWE::Instance instance(VK_MAKE_API_VERSION(0, 1, 4, 0), requiredExtensions, optionalExtensions);

    //once the instance is created, create a surface
    //the surface needs to be known, to check if the physical devices can render to it
    //potentially, could make headless applications, but i don't personally have interest in supporting that at the moment
    EWE::Window window{instance, 800, 600, "Example Window"};


    std::vector<EWE::RequestedExtension> requestedExtensions{
        EWE::RequestedExtension{VK_KHR_SWAPCHAIN_EXTENSION_NAME, true, 0},
        EWE::RequestedExtension{VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME, true, 0},
        EWE::RequestedExtension{VK_EXT_MESH_SHADER_EXTENSION_NAME, true, 0},
        EWE::RequestedExtension{VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME, true, 0}
    };
    constexpr uint32_t rounded_down_vulkan_version = VK_MAKE_VERSION(1, 4, 0);
    EWE::DeviceSpecializer<
        rounded_down_vulkan_version,
        FeatureManager<rounded_down_vulkan_version, 
            VkPhysicalDeviceExtendedDynamicState3FeaturesEXT,
            VkPhysicalDeviceMeshShaderFeaturesEXT,
            VkPhysicalDeviceDescriptorIndexingFeatures,

        >,
        PropertyManager<rounded_down_vulkan_version,
            VkPhysicalDeviceMeshShaderPropertiesEXT,
            VkPhysicalDeviceDescriptorIndexingProperties

        >
    > deviceData{requestedExtensions};
    
    auto& dynState3 = deviceData.GetFeature<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>();
    dynState3.extendedDynamicState3ColorBlendEnable = VK_TRUE;
    dynState3.extendedDynamicState3ColorBlendEquation = VK_TRUE;
    dynState3.extendedDynamicState3ColorWriteMask = VK_TRUE;
    
    auto& meshShaderFeatures = deviceData.GetFeature<VkPhysicalDeviceMeshShaderFeaturesEXT>();
    meshShaderFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;
    meshShaderFeatures.meshShader = VK_TRUE;
    meshShaderFeatures.taskShader = VK_TRUE;
    
    auto& features2 = deviceData.GetFeature<VkPhysicalDeviceFeatures2>();
    features2.features.samplerAnisotropy = VK_TRUE;
    features2.features.geometryShader = VK_TRUE;
    features2.features.wideLines = VK_TRUE;
    //features2.features.tessellationShader = VK_TRUE;

    auto& indexingFeatures = deviceData.GetFeature<VkPhysicalDeviceDescriptorIndexingFeatures>();
    indexingFeatures.runtimeDescriptorArray = VK_TRUE;
    indexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
    indexingFeatures.descriptorBindingVariableDescriptorCount = VK_TRUE;
    indexingFeatures.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
    indexingFeatures.descriptorBindingUpdateUnusedWhilePending = VK_TRUE;


    //i need a way to request device features

    EWE::LogicalDevice logicalDevice(instance, window.surface, extensions);

    EWE::Swapchain swapchain{logicalDevice};
    //from here, create the render graph


    return 0;
}