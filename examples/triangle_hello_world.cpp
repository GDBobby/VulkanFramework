//example
#include "EightWinds/VulkanHeader.h"
#include "EightWinds/Window.h"
#include "EightWinds/LogicalDevice.h"
#include "EightWinds/Swapchain.h"

#include "EightWinds/Pipeline/Layout.h"
#include "EightWinds/Pipeline/PipelineBase.h"
#include "EightWinds/Pipeline/Graphics.h"

#include "EightWinds/Backend/DeviceSpecialization/Extensions.h"
#include "EightWinds/Backend/DeviceSpecialization/DeviceSpecialization.h"
#include "EightWinds/Backend/DeviceSpecialization/FeatureProperty.h"

#include "EightWinds/Shader.h"

#include <cstdint>
#include <cstdio>
#include <cassert>
#include <filesystem>
#include <chrono>


    //the variant is useless, but VK_MAKE_VERSION is deprecated
constexpr uint32_t application_wide_vk_version = VK_MAKE_API_VERSION(0, 1, 4, 0);

    //weights are at minimum 1,
    //so if a property has a weight of 1, and a quanity of 32k, it'll be worth 64k
    //other scores need to be high to match.
    //for example, this mesh shader extension is going to be scored at 100k
constexpr uint64_t max_uint64_t = UINT64_MAX;
    //it's a uint64, so be liberal with points

//this really sucks, i need a better way around it
constexpr EWE::ConstEvalStr swapchainExt{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
constexpr EWE::ConstEvalStr dynState3Ext{ VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME };
constexpr EWE::ConstEvalStr meshShaderExt{ VK_EXT_MESH_SHADER_EXTENSION_NAME };
constexpr EWE::ConstEvalStr descriptorIndexingExt{ VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME };


using Example_ExtensionManager = EWE::ExtensionManager<application_wide_vk_version,
    EWE::ExtensionEntry<swapchainExt, true, 0>,
    EWE::ExtensionEntry<dynState3Ext, true, 0>,
    EWE::ExtensionEntry<meshShaderExt, false, 100000>,
    EWE::ExtensionEntry<descriptorIndexingExt, true, 0>
>;


constexpr uint32_t rounded_down_vulkan_version = EWE::RoundDownVkVersion(application_wide_vk_version);

using Example_FeatureManager = EWE::FeatureManager<rounded_down_vulkan_version,
    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT,
    VkPhysicalDeviceMeshShaderFeaturesEXT,
    VkPhysicalDeviceDescriptorIndexingFeatures
>;

using Example_PropertyManager = EWE::PropertyManager<rounded_down_vulkan_version,
    VkPhysicalDeviceMeshShaderPropertiesEXT,
    VkPhysicalDeviceDescriptorIndexingProperties

>;

using DeviceSpec = EWE::DeviceSpecializer<
    rounded_down_vulkan_version,
    Example_ExtensionManager,
    Example_FeatureManager,
    Example_PropertyManager
>;



struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;

    [[nodiscard]] explicit SwapChainSupportDetails(VkPhysicalDevice device, VkSurfaceKHR surface) noexcept {
        EWE::EWE_VK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR, device, surface, &capabilities);
        uint32_t formatCount;
        EWE::EWE_VK(vkGetPhysicalDeviceSurfaceFormatsKHR, device, surface, &formatCount, nullptr);

        if (formatCount != 0) {
            formats.resize(formatCount);
            EWE::EWE_VK(vkGetPhysicalDeviceSurfaceFormatsKHR, device, surface, &formatCount, formats.data());
        }

        uint32_t presentModeCount;
        EWE::EWE_VK(vkGetPhysicalDeviceSurfacePresentModesKHR, device, surface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            presentModes.resize(presentModeCount);
            EWE::EWE_VK(vkGetPhysicalDeviceSurfacePresentModesKHR, device, surface, &presentModeCount, presentModes.data());
        }
    }


    [[nodiscard]] bool Adequate() const { return !formats.empty() && !presentModes.empty(); }
};

int main() {


    std::vector<const char*> requiredExtensions{
#ifdef _DEBUG
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,

#endif
    };
    if (!glfwInit()) {
        printf("failed to glfw init\n");
    }
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    assert(glfwExtensionCount > 0 && "not supporting headless");
    assert(glfwExtensions != nullptr);

    for (uint32_t i = 0; i < glfwExtensionCount; ++i) {
        requiredExtensions.push_back(glfwExtensions[i]);
    }

    std::unordered_map<std::string, bool> optionalExtensions{};

    EWE::Instance instance(application_wide_vk_version, requiredExtensions, optionalExtensions);

    //once the instance is created, create a surface
    //the surface needs to be known, to check if the physical devices can render to it
    //potentially, could make headless applications, but i don't personally have interest in supporting that at the moment
    EWE::Window window{ instance, 800, 600, "Example Window" };


    DeviceSpec specDev{};

    //vk::CppType<VkPhysicalDeviceMeshShaderFeaturesEXT>::Type meshCPPTypeObject;
    //VULKAN_HPP_CPP_VERSION;

    auto& dynState3 = specDev.GetFeature<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>();
    dynState3.extendedDynamicState3ColorBlendEnable = VK_TRUE;
    dynState3.extendedDynamicState3ColorBlendEquation = VK_TRUE;
    dynState3.extendedDynamicState3ColorWriteMask = VK_TRUE;

    auto& meshShaderFeatures = specDev.GetFeature<VkPhysicalDeviceMeshShaderFeaturesEXT>();
    meshShaderFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;
    meshShaderFeatures.meshShader = VK_TRUE;
    meshShaderFeatures.taskShader = VK_TRUE;

    auto& features2 = specDev.GetFeature<VkPhysicalDeviceFeatures2>();
    features2.features.samplerAnisotropy = VK_TRUE;
    features2.features.geometryShader = VK_TRUE;
    features2.features.wideLines = VK_TRUE;
    //features2.features.tessellationShader = VK_TRUE;

    auto& indexingFeatures = specDev.GetFeature<VkPhysicalDeviceDescriptorIndexingFeatures>();
    indexingFeatures.runtimeDescriptorArray = VK_TRUE;
    indexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
    indexingFeatures.descriptorBindingVariableDescriptorCount = VK_TRUE;
    indexingFeatures.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
    indexingFeatures.descriptorBindingUpdateUnusedWhilePending = VK_TRUE;

    uint32_t deviceCount;
    EWE_VK(vkEnumeratePhysicalDevices, instance, &deviceCount, nullptr);
    std::vector<VkPhysicalDevice> all_detected_physical_devices(deviceCount);
    if (deviceCount == 0) {
        printf("0 devices found, exiting\n");
        return -1;
    }
    EWE_VK(vkEnumeratePhysicalDevices, instance, &deviceCount, all_detected_physical_devices.data());

    auto evaluatedDevices = specDev.ScorePhysicalDevices(instance.instance);

    if (!evaluatedDevices[0].passedRequirements) {
        printf("highest score device failed requirements, exiting\n");
        return -1;
    }

    EWE::PhysicalDevice physicalDevice{ instance, evaluatedDevices[0].device, window.surface };

    EWE::LogicalDevice logicalDevice = specDev.ConstructDevice(
        evaluatedDevices[0],
        //i want physicaldevice to be moved, but i might just construct it inside the logicaldevice
        //the main thing is i just dont want to repopulate the queues
        std::forward<EWE::PhysicalDevice>(physicalDevice),
        nullptr
    );

    EWE::Queue* renderQueue = nullptr;
    for (auto& queue : logicalDevice.queues) {
        if (queue.family.SupportsSurfacePresent() && queue.family.SupportsGraphics()) {
            renderQueue = &queue;
            break;
        }
    }
    if (renderQueue == nullptr) {
        printf("failed to find a render queue, exiting\n");
        return -1;
    }
    

    EWE::Swapchain swapchain{ logicalDevice, window, *renderQueue };

    printf("current dir - %s\n", std::filesystem::current_path().string().c_str());

    //need to fix htis
    std::filesystem::current_path("C:/Projects/VulkanFramework");
    printf("current dir - %s\n", std::filesystem::current_path().string().c_str());
    EWE::Framework framework(logicalDevice);
    framework.properties = specDev.GetProperty<VkPhysicalDeviceProperties2>().properties;

    auto* triangle_vert = framework.shaderFactory.GetShader("examples/common/shaders/basic.vert.spv");
    auto* triangle_frag = framework.shaderFactory.GetShader("examples/common/shaders/basic.frag.spv");


    EWE::PipeLayout triangle_layout(framework, std::initializer_list<EWE::Shader*>{ triangle_vert, triangle_frag });

    //create a renderpass here
    //EWE::PipelineConfigInfo configInfo;
    std::vector<VkDynamicState> dynamicStates{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    VkPipelineRenderingCreateInfo renderingCreateInfo{};
    renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    renderingCreateInfo.pNext = nullptr;
    renderingCreateInfo.colorAttachmentCount = 1;


    
    EWE::CommandPool renderCmdPool{logicalDevice, *renderQueue, false};
    std::vector<VkCommandBuffer> cmdBufVector(2, VK_NULL_HANDLE);

    VkCommandBufferAllocateInfo cmdBufAllocInfo{};
    cmdBufAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdBufAllocInfo.pNext = nullptr;
    cmdBufAllocInfo.commandBufferCount = 2;
    cmdBufAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdBufAllocInfo.commandPool = renderCmdPool.commandPool;
    
    EWE::EWE_VK(vkAllocateCommandBuffers, logicalDevice.device, &cmdBufAllocInfo, cmdBufVector.data());
    renderCmdPool.allocatedBuffers += 2;

    std::vector<EWE::CommandBuffer> commandBuffers{};
    commandBuffers.emplace_back(renderCmdPool, cmdBufVector[0]);
    commandBuffers.emplace_back(renderCmdPool, cmdBufVector[1]);

    uint8_t currentFrame = 0;
    auto timeBegin = std::chrono::high_resolution_clock::now();


    //from here, create the render graph

    //passconfig should be using a full rendergraph setup
    EWE::PipelinePassConfig passConfig;
    passConfig.SetDefaults();
    EWE::PipelineObjectConfig objectConfig;
    objectConfig.SetDefaults();

    std::vector<VkDynamicState> dynamicState{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    EWE::GraphicsPipeline triangle_pipeline{ logicalDevice, 0, &triangle_layout, passConfig, objectConfig, dynamicState };

    uint8_t frameIndex = 0;
    VkCommandBufferBeginInfo cmdBeginInfo{};
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginInfo.pNext = nullptr;
    cmdBeginInfo.pInheritanceInfo = nullptr;
    cmdBeginInfo.flags = 0;

    //this will also get filled out by the rendergraph
    VkRenderingInfo renderingInfo{};
    renderingInfo.
    while (true) {
        EWE::CommandBuffer& currentCmdBuf = commandBuffers[currentFrame];
        EWE::EWE_VK(vkBeginCommandBuffer, currentCmdBuf.cmdBuf, &cmdBeginInfo);
        vkCmdBeginRendering(currentCmdBuf.cmdBuf, &renderingInfo);
    }


    printf("returning successfully\n");

    delete triangle_vert;
    delete triangle_frag;


    return 0;
}