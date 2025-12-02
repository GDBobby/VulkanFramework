//example
#include "EightWinds/VulkanHeader.h"
#include "EightWinds/Window.h"
#include "EightWinds/LogicalDevice.h"
#include "EightWinds/Swapchain.h"

#include "EightWinds/Pipeline/Layout.h"
#include "EightWinds/Pipeline/PipelineBase.h"
#include "EightWinds/Pipeline/Graphics.h"
#include "EightWinds/Shader.h"

#include "EightWinds/Backend/DeviceSpecialization/Extensions.h"
#include "EightWinds/Backend/DeviceSpecialization/DeviceSpecialization.h"
#include "EightWinds/Backend/DeviceSpecialization/FeatureProperty.h"


#include "EightWinds/Command/Record.h"
#include "EightWinds/Command/Execute.h"
#include "EightWinds/RenderGraph/GPUTask.h"
#include "EightWinds/GlobalPushConstant.h"

#include "EightWinds/Image.h"

#include <cstdint>
#include <cstdio>
#include <cassert>
#include <filesystem>
#include <chrono>

void PrintAllExtensions(VkPhysicalDevice physicalDevice) {
    uint32_t extCount = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, nullptr);
    std::vector<VkExtensionProperties> extProps(extCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, extProps.data());
    printf("available extensions --\n");
    for (auto& prop : extProps) {
        printf("\t%s\n", prop.extensionName);
    }
}


//https://docs.vulkan.org/refpages/latest/refpages/source/VK_EXT_device_address_binding_report.html
//vk_ext_device_fault
//^ potential debugging extension

    //the variant is useless, but VK_MAKE_VERSION is deprecated
constexpr uint32_t application_wide_vk_version = VK_MAKE_API_VERSION(0, 1, 4, 0);

    //weights are at minimum 1,
    //so if a property has a weight of 1, and a quanity of 32k, it'll be worth 64k
    //other scores need to be high to match.
    //for example, this mesh shader extension is going to be scored at 100k
constexpr uint64_t max_uint64_t = UINT64_MAX;
    //it's a uint64, so be liberal with points

//this really sucks, i need a better way around it
constexpr EWE::ConstEvalStr swapchainExt{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
//https://docs.vulkan.org/refpages/latest/refpages/source/VK_EXT_extended_dynamic_state3.html
constexpr EWE::ConstEvalStr dynState3Ext{ VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME };
constexpr EWE::ConstEvalStr meshShaderExt{ VK_EXT_MESH_SHADER_EXTENSION_NAME };
constexpr EWE::ConstEvalStr deviceFaultExt{ VK_EXT_DEVICE_FAULT_EXTENSION_NAME };
//this requires the instance extension VK_EXT_debug_utils. i don't know how to make that association cleanly
constexpr EWE::ConstEvalStr dabReportExt{ VK_EXT_DEVICE_ADDRESS_BINDING_REPORT_EXTENSION_NAME }; 


using Example_ExtensionManager = EWE::ExtensionManager<application_wide_vk_version,
    EWE::ExtensionEntry<swapchainExt, true, 0>,
    EWE::ExtensionEntry<dynState3Ext, true, 0>,
    EWE::ExtensionEntry<meshShaderExt, false, 100000>,
    EWE::ExtensionEntry<deviceFaultExt, true, 0>,
    EWE::ExtensionEntry<dabReportExt, true, 0>
>;


constexpr uint32_t rounded_down_vulkan_version = EWE::RoundDownVkVersion(application_wide_vk_version);

using Example_FeatureManager = EWE::FeatureManager<rounded_down_vulkan_version,
    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT,
    VkPhysicalDeviceMeshShaderFeaturesEXT,
    VkPhysicalDeviceFaultFeaturesEXT,
    VkPhysicalDeviceAddressBindingReportFeaturesEXT
>;

using Example_PropertyManager = EWE::PropertyManager<rounded_down_vulkan_version,
    VkPhysicalDeviceMeshShaderPropertiesEXT
    //descriptor indexing properties in vulkan12properties?

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

    VK_HEADER_VERSION_COMPLETE;

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
    requiredExtensions.push_back(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
    

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
    //features2.features.geometryShader = VK_TRUE;
    features2.features.wideLines = VK_TRUE;
    features2.features.shaderInt64 = VK_TRUE;
    
    auto& features12 = specDev.GetFeature<VkPhysicalDeviceVulkan12Features>();
    features12.scalarBlockLayout = VK_TRUE;
    features12.bufferDeviceAddress = VK_TRUE;
    features12.descriptorBindingPartiallyBound = VK_TRUE;
    features12.runtimeDescriptorArray = VK_TRUE;
    features12.descriptorBindingVariableDescriptorCount = VK_TRUE;
    features12.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
    features12.descriptorBindingUpdateUnusedWhilePending = VK_TRUE;

    auto& features13 = specDev.GetFeature<VkPhysicalDeviceVulkan13Features>();
    features13.dynamicRendering = VK_TRUE;

    auto& devFaultFeatures = specDev.GetFeature<VkPhysicalDeviceFaultFeaturesEXT>();
    devFaultFeatures.deviceFault = VK_TRUE;
    devFaultFeatures.deviceFaultVendorBinary = VK_TRUE;

    auto& dabReportFeatures = specDev.GetFeature<VkPhysicalDeviceAddressBindingReportFeaturesEXT>();
    dabReportFeatures.reportAddressBinding = VK_TRUE;

    auto evaluatedDevices = specDev.ScorePhysicalDevices(instance.instance);

    if (!evaluatedDevices[0].passedRequirements) {
        printf("highest score device failed requirements, exiting\n");
        printf("device count - %zu\n", evaluatedDevices.size());
#if EWE_DEBUG_BOOL
        for(uint8_t i = 0; i < evaluatedDevices.size(); i++) {
            auto& evDev = evaluatedDevices[i];
            printf("results[%d] - %s --- %d - %zu\n", i, evDev.name.c_str(), evDev.passedRequirements, evDev.score);
            const uint32_t variant_version = evDev.api_version >> 29;
            const uint32_t major_version = (evDev.api_version - (variant_version << 29)) >> 22;
            const uint32_t minor_version = (evDev.api_version - (variant_version << 29) - (major_version << 22)) >> 12;
            const uint32_t patch_version = (evDev.api_version - (variant_version << 29) - (major_version << 22) - (minor_version << 12));

            printf("api version - %d.%d.%d.%d\n", variant_version, major_version, minor_version, patch_version);

            for(auto& fp : evDev.failureReport){
                printf("\t\tfp - %s\n", fp.c_str());
            }
        }
#endif
        return -1;
    }
    else{
        auto& evDev = evaluatedDevices[0];
        const uint32_t variant_version = evDev.api_version >> 29;
        const uint32_t major_version = (evDev.api_version - (variant_version << 29)) >> 22;
        const uint32_t minor_version = (evDev.api_version - (variant_version << 29) - (major_version << 22)) >> 12;
        const uint32_t patch_version = (evDev.api_version - (variant_version << 29) - (major_version << 22) - (minor_version << 12));

        printf("api version - %d.%d.%d.%d\n", variant_version, major_version, minor_version, patch_version);
    }

    EWE::PhysicalDevice physicalDevice{ instance, evaluatedDevices[0].device, window.surface };
    //PrintAllExtensions(physicalDevice.device);

    /*
    quick notes, on the pnext chain
    i was waiting to see how it evolved a bit, and here's my current understanding
    if the extension is enabled, then I add feature struct to VkDeviceCreateInfo::pnext
    every feature so far has the following line
    
    If the application wishes to use a VkDevice with any features described by VkPhysicalDeviceMeshShaderFeaturesEXT, it must add an instance of the structure, with the desired feature members set to VK_TRUE, to the pNext chain of VkDeviceCreateInfo when creating the VkDevice.

    once that is done, vkGetPhysicalDeviceFeatures2 can be polled to check whats available in the driver/hardware
    i dont really know how to build a robust requirement chain here, maybe use device scoring
    BUT, features are DEPENDENT on extensions, but not always the other way around
    
    currently, i think i could make an extension struct, consteval
    give it a name and a type for feature, or void if no feature
    i dont think theres any features that arent dependent on an extension
    */

    //the stypes and pnexts were populated when scoring the devices
    VkBaseInStructure* pNextChain = reinterpret_cast<VkBaseInStructure*>(&specDev.features.base);
    //i shouldnt need to rebuild this, something is wrong
    auto& feat12 = specDev.GetFeature<VkPhysicalDeviceVulkan12Features>();
    auto addr = reinterpret_cast<std::size_t>(&feat12);
    printf("base addr - %zx\n", &specDev.features.base);
    uint16_t whichAddr = 0;
    auto addr_print_func = [&whichAddr](auto& feat) {
        printf("addr[%u] - %zx\n", whichAddr++, &feat);
    };
    specDev.features.features.ForEach(addr_print_func);

    EWE::LogicalDevice logicalDevice = specDev.ConstructDevice(
        evaluatedDevices[0],
        //i want physicaldevice to be moved, but i might just construct it inside the logicaldevice
        //the main thing is i just dont want to repopulate the queues
        std::forward<EWE::PhysicalDevice>(physicalDevice),
        pNextChain,
        application_wide_vk_version,
        VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT
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
    //framework.graphicsLibraryEnabled = specDev.GetExtensionIndex(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    framework.deviceFaultEnabled = specDev.extension_support[specDev.GetExtensionIndex(VK_EXT_DEVICE_FAULT_EXTENSION_NAME)];
    framework.meshShadersEnabled = specDev.extension_support[specDev.GetExtensionIndex(VK_EXT_MESH_SHADER_EXTENSION_NAME)];

    auto* triangle_vert = framework.shaderFactory.GetShader("examples/common/shaders/basic.vert.spv");
    auto* triangle_frag = framework.shaderFactory.GetShader("examples/common/shaders/basic.frag.spv");

    EWE::PipeLayout triangle_layout(framework, std::initializer_list<EWE::Shader*>{ triangle_vert, triangle_frag });

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

    //if either of these formats are changed, passConfig needs to be changed as well. these just happen to match the defaults
    VkFormat colorFormat = VK_FORMAT_R8G8B8A8_UNORM;
    VkFormat depthFormat = VK_FORMAT_D16_UNORM;

    //from here, create the render graph

    //passconfig should be using a full rendergraph setup
    EWE::PipelinePassConfig passConfig;
    passConfig.SetDefaults();
    EWE::PipelineObjectConfig objectConfig;
    objectConfig.SetDefaults();

    std::vector<VkDynamicState> dynamicState{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    EWE::GraphicsPipeline triangle_pipeline{ logicalDevice, 0, &triangle_layout, passConfig, objectConfig, dynamicState };


    EWE::PerFlight<EWE::Image> colorAttachmentImages{logicalDevice};
    EWE::PerFlight<EWE::Image> depthAttachmentImages{logicalDevice};

    //most of the pnext seem useless. some other useful stuff, 
    /*
    VkOpaqueCaptureDescriptorDataCreateInfoEXT,
    VkExternalMemoryImageCreateInfo
    VkImageFormatListCreateInfo,
    */
    for (uint8_t i = 0; i < 2; i++) {
        colorAttachmentImages[i].arrayLayers = 1;
            depthAttachmentImages[i].arrayLayers = 1;
        colorAttachmentImages[i].extent = { window.screenDimensions.width, window.screenDimensions.height, 1 };
            depthAttachmentImages[i].extent = { window.screenDimensions.width, window.screenDimensions.height, 1 };
        colorAttachmentImages[i].mipLevels = 1;
            depthAttachmentImages[i].mipLevels = 1;
        colorAttachmentImages[i].owningQueue = renderQueue;
            depthAttachmentImages[i].owningQueue = renderQueue;
        colorAttachmentImages[i].samples = VK_SAMPLE_COUNT_1_BIT;
            depthAttachmentImages[i].samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachmentImages[i].tiling = VK_IMAGE_TILING_OPTIMAL;
            depthAttachmentImages[i].tiling = VK_IMAGE_TILING_OPTIMAL;
        colorAttachmentImages[i].type = VK_IMAGE_TYPE_2D;
            depthAttachmentImages[i].type = VK_IMAGE_TYPE_2D;
    }

    VmaAllocationCreateInfo vmaAllocCreateInfo{};
    vmaAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    //if(imageCreateInfo.width * height > some amount){
    vmaAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT | VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT;
    //}
    for (auto& cai : colorAttachmentImages) {
        cai.format = VK_FORMAT_R8G8B8A8_UNORM;
        cai.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        cai.Create(vmaAllocCreateInfo);
    }
    for (auto& dai : depthAttachmentImages) {
        dai.format = VK_FORMAT_D16_UNORM;
        dai.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        dai.Create(vmaAllocCreateInfo);
    }

    VkImageViewCreateInfo imgViewCreateInfo{};
    imgViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imgViewCreateInfo.pNext = nullptr;
    imgViewCreateInfo.flags = 0;
    imgViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imgViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imgViewCreateInfo.subresourceRange.baseMipLevel = 0;
    imgViewCreateInfo.subresourceRange.levelCount = 1;
    imgViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imgViewCreateInfo.subresourceRange.layerCount = 1;

    EWE::PerFlight<VkImageView> colorAttViews;
    EWE::PerFlight<VkImageView> depthAttViews;
    imgViewCreateInfo.format = colorFormat;
    imgViewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
    for (uint8_t i = 0; i < EWE::max_frames_in_flight; i++) {
        imgViewCreateInfo.image = colorAttachmentImages[i].image;
        EWE::EWE_VK(vkCreateImageView, logicalDevice.device, &imgViewCreateInfo, nullptr, &colorAttViews[i]);
    }

    imgViewCreateInfo.format = depthFormat;
    imgViewCreateInfo.components = {VK_COMPONENT_SWIZZLE_R};
    imgViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    for (uint8_t i = 0; i < EWE::max_frames_in_flight; i++) {
        imgViewCreateInfo.image = depthAttachmentImages[i].image;
        EWE::EWE_VK(vkCreateImageView, logicalDevice.device, &imgViewCreateInfo, nullptr, &depthAttViews[i]);
    }

    //this will also get filled out by the rendergraph
    VkRenderingInfo renderingInfo{};

    EWE::CommandRecord renderRecord{};
    auto* def_beginRender = renderRecord.BeginRender();
    auto* def_pipe = renderRecord.BindPipeline();
    auto* def_vp_scissor = renderRecord.SetViewportScissor();
    //auto* def_desc = cmdRecord.BindDescriptor();
    uint32_t pushIndex = renderRecord.Push();
    auto* def_draw = renderRecord.Draw();
    renderRecord.EndRender();
    EWE::GPUTask gpuTask = renderRecord.Compile(logicalDevice);

    def_beginRender->data->colorAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    def_beginRender->data->colorAttachmentInfo.pNext = nullptr;
    def_beginRender->data->colorAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    def_beginRender->data->colorAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    def_beginRender->data->colorAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    def_beginRender->data->colorAttachmentInfo.clearValue.color.float32[0] = 0.f;
    def_beginRender->data->colorAttachmentInfo.clearValue.color.float32[1] = 0.f;
    def_beginRender->data->colorAttachmentInfo.clearValue.color.float32[2] = 0.f;
    def_beginRender->data->colorAttachmentInfo.clearValue.color.float32[3] = 0.f;
    def_beginRender->data->colorAttachmentInfo.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    def_beginRender->data->colorAttachmentInfo.resolveImageView = VK_NULL_HANDLE;
    def_beginRender->data->colorAttachmentInfo.resolveMode = VK_RESOLVE_MODE_NONE;

    def_beginRender->data->depthAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    def_beginRender->data->depthAttachmentInfo.pNext = nullptr;
    def_beginRender->data->depthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    def_beginRender->data->depthAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    def_beginRender->data->depthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    def_beginRender->data->depthAttachmentInfo.clearValue.depthStencil.depth = 0.f;
    def_beginRender->data->depthAttachmentInfo.clearValue.depthStencil.stencil = 0; //idk what to set this to tbh
    def_beginRender->data->depthAttachmentInfo.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    def_beginRender->data->depthAttachmentInfo.resolveImageView = VK_NULL_HANDLE;
    def_beginRender->data->depthAttachmentInfo.resolveMode = VK_RESOLVE_MODE_NONE;

    def_beginRender->data->renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    def_beginRender->data->renderingInfo.pNext = nullptr;
    def_beginRender->data->renderingInfo.colorAttachmentCount = 1;
    def_beginRender->data->renderingInfo.flags = 0;
    def_beginRender->data->renderingInfo.layerCount = 1;
    def_beginRender->data->renderingInfo.pColorAttachments = &def_beginRender->data->colorAttachmentInfo;
    def_beginRender->data->renderingInfo.pDepthAttachment = &def_beginRender->data->depthAttachmentInfo;
    def_beginRender->data->renderingInfo.renderArea = window.screenDimensions;
    def_beginRender->data->renderingInfo.viewMask = 0;

    *def_pipe->data = reinterpret_cast<EWE::Pipeline*>(&triangle_pipeline);
    def_vp_scissor->data->scissor = window.screenDimensions;
    def_vp_scissor->data->viewport.x = 0.f;
    def_vp_scissor->data->viewport.y = static_cast<float>(window.screenDimensions.height);
    def_vp_scissor->data->viewport.width = static_cast<float>(window.screenDimensions.width);
    def_vp_scissor->data->viewport.height = -static_cast<float>(window.screenDimensions.height);
    def_vp_scissor->data->viewport.minDepth = 0.0f;
    def_vp_scissor->data->viewport.maxDepth = 1.f;

    VmaAllocationCreateInfo vmaAllocInfo{};
    vmaAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    vmaAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    vmaAllocInfo.memoryTypeBits = 0;
    vmaAllocInfo.pool = VK_NULL_HANDLE;
    vmaAllocInfo.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    vmaAllocInfo.requiredFlags = 0;
    vmaAllocInfo.priority = 1.f;
    vmaAllocInfo.pUserData = nullptr;

    struct TriangleVertex {
        float pos[2]; //xy
        float color[3]; //rgb
    };
    EWE::Buffer vertex_buffer{framework, sizeof(TriangleVertex) * 3, 1, vmaAllocInfo, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT };
    TriangleVertex* mappedData = reinterpret_cast<TriangleVertex*>(vertex_buffer.Map());

    mappedData[0].pos[0] = -0.5f;
    mappedData[0].pos[1] = -0.5f;

    mappedData[1].pos[0] = 0.f;
    mappedData[1].pos[1] = 0.5f;

    mappedData[2].pos[0] = 0.5f;
    mappedData[2].pos[1] = 0.5f;

    mappedData[0].color[0] = 1.f;
    mappedData[0].color[1] = 0.f;
    mappedData[0].color[2] = 0.f;

    mappedData[1].color[0] = 0.f;
    mappedData[1].color[1] = 1.f;
    mappedData[1].color[2] = 0.f;

    mappedData[2].color[0] = 0.f;
    mappedData[2].color[1] = 0.f;
    mappedData[2].color[2] = 1.f;

    vertex_buffer.Flush();
    vertex_buffer.Unmap();

    gpuTask.UseBuffer(&vertex_buffer, pushIndex, 0, false);

    def_draw->data->firstInstance = 0;
    def_draw->data->firstVertex = 0;
    def_draw->data->instanceCount = 1;
    def_draw->data->vertexCount = 3;

    VkSubmitInfo submitInfo{};
    submitInfo.commandBufferCount = 1;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.pSignalSemaphores = nullptr;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = nullptr;

    //auto timeBegin = std::chrono::high_resolution_clock::now();
    VkCommandBufferBeginInfo cmdBeginInfo{};
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginInfo.pNext = nullptr;
    cmdBeginInfo.pInheritanceInfo = nullptr;
    cmdBeginInfo.flags = 0;
    uint8_t frameIndex = 1;
    try {
        while (true) {
            def_beginRender->data->colorAttachmentInfo.imageView = colorAttViews[frameIndex];
            def_beginRender->data->depthAttachmentInfo.imageView = depthAttViews[frameIndex];

            EWE::CommandBuffer& currentCmdBuf = commandBuffers[frameIndex];
            //EWE::EWE_VK(vkBeginCommandBuffer, currentCmdBuf.cmdBuf, &cmdBeginInfo);
            currentCmdBuf.Begin(cmdBeginInfo);
            gpuTask.Execute(currentCmdBuf);

            //EWE::EWE_VK(vkEndCommandBuffer, currentCmdBuf);
            currentCmdBuf.End();
            submitInfo.pCommandBuffers = &currentCmdBuf.cmdBuf;
            //EWE::EWE_VK(vkQueueSubmit, *renderQueue, 1, &submitInfo, VK_NULL_HANDLE);
            renderQueue->Submit(1, &submitInfo, VK_NULL_HANDLE);
            frameIndex = (frameIndex + 1) % EWE::max_frames_in_flight;
        }
    }
    catch (EWE::EWEException& except) {
        framework.HandleVulkanException(except);
    }


    printf("returning successfully\n");

    delete triangle_vert;
    delete triangle_frag;


    return 0;
}