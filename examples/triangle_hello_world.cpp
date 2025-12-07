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
#include "EightWinds/RenderGraph/TaskBridge.h"
#include "EightWinds/RenderGraph/RenderGraph.h"

#include "EightWinds/GlobalPushConstant.h"

#include "EightWinds/Image.h"
#include "EightWinds/ImageView.h"

#include <cstdint>
#if EWE_DEBUG_BOOL
#include <cstdio>
#include <cassert>
#endif
#include <filesystem>
#include <chrono>
#include <array>

#if EWE_DEBUG_BOOL
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
#endif


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
    EWE::ExtensionEntry<deviceFaultExt, false, 0>,
    EWE::ExtensionEntry<dabReportExt, false, 0>
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
#if EWE_DEBUG_BOOL
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,

#endif
    };
    if (!glfwInit()) {
#if EWE_DEBUG_BOOL
        printf("failed to glfw init\n");
#endif
        return -1;
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
    features13.synchronization2 = VK_TRUE;

    auto& devFaultFeatures = specDev.GetFeature<VkPhysicalDeviceFaultFeaturesEXT>();
    devFaultFeatures.deviceFault = VK_TRUE;
    devFaultFeatures.deviceFaultVendorBinary = VK_TRUE;

    auto& dabReportFeatures = specDev.GetFeature<VkPhysicalDeviceAddressBindingReportFeaturesEXT>();
    dabReportFeatures.reportAddressBinding = VK_TRUE;

    auto evaluatedDevices = specDev.ScorePhysicalDevices(instance);

    if (!evaluatedDevices[0].passedRequirements) {
#if EWE_DEBUG_BOOL
        printf("highest score device failed requirements, exiting\n");
        printf("device count - %zu\n", evaluatedDevices.size());
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
        std::this_thread::sleep_for(std::chrono::seconds(5));

        return -1;
    }
#if EWE_DEBUG_BOOL
    else{
        auto& evDev = evaluatedDevices[0];
        const uint32_t variant_version = evDev.api_version >> 29;
        const uint32_t major_version = (evDev.api_version - (variant_version << 29)) >> 22;
        const uint32_t minor_version = (evDev.api_version - (variant_version << 29) - (major_version << 22)) >> 12;
        const uint32_t patch_version = (evDev.api_version - (variant_version << 29) - (major_version << 22) - (minor_version << 12));

        printf("api version - %d.%d.%d.%d\n", variant_version, major_version, minor_version, patch_version);
    }
#endif

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

    EWE::LogicalDevice logicalDevice = specDev.ConstructDevice(
        evaluatedDevices[0],
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
#if EWE_DEBUG_BOOL
        printf("failed to find a render queue, exiting\n");
#endif
        std::this_thread::sleep_for(std::chrono::seconds(5));
        return -1;
    }
    

    EWE::Swapchain swapchain{ logicalDevice, window, *renderQueue };

#if EWE_DEBUG_BOOL
    printf("current dir - %s\n", std::filesystem::current_path().string().c_str());
#endif

    //need to fix htis. its something with my windows debugger
#ifdef _WIN32
    std::filesystem::current_path("C:/Projects/VulkanFramework");
#else
    auto current_working_directory = std::filesystem::current_path();
    auto parentStem = current_working_directory.parent_path().stem();
    if(parentStem == "build"){
        current_working_directory = current_working_directory.parent_path().parent_path();
#if EWE_DEBUG_BOOL
        printf("build redacted working dir - %s\n", current_working_directory.string().c_str());
#endif
    }
    std::filesystem::current_path(current_working_directory);
#endif
#if EWE_DEBUG_BOOL
    printf("current dir - %s\n", std::filesystem::current_path().string().c_str());
#endif
    EWE::Framework framework(logicalDevice);
    framework.properties = specDev.GetProperty<VkPhysicalDeviceProperties2>().properties;
    //framework.graphicsLibraryEnabled = specDev.GetExtensionIndex(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
    framework.deviceFaultEnabled = specDev.extension_support[specDev.GetExtensionIndex(VK_EXT_DEVICE_FAULT_EXTENSION_NAME)];
    framework.meshShadersEnabled = specDev.extension_support[specDev.GetExtensionIndex(VK_EXT_MESH_SHADER_EXTENSION_NAME)];

    //if either of these formats are changed, passConfig needs to be changed as well. these just happen to match the defaults
    VkFormat colorFormat = VK_FORMAT_R8G8B8A8_UNORM;
    VkFormat depthFormat = VK_FORMAT_D16_UNORM;

    //from here, create the render graph


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
    vmaAllocCreateInfo.flags = static_cast<VmaAllocationCreateFlags>(VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT) | static_cast<VmaAllocationCreateFlags>(VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT);
    //}
    for (auto& cai : colorAttachmentImages) {
        cai.format = colorFormat;
        cai.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        cai.Create(vmaAllocCreateInfo);
    }
    for (auto& dai : depthAttachmentImages) {
        dai.format = depthFormat;
        dai.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        dai.Create(vmaAllocCreateInfo);
    }
#if EWE_DEBUG_NAMING
    colorAttachmentImages[0].SetName("cai 0");
    colorAttachmentImages[1].SetName("cai 1");
    depthAttachmentImages[0].SetName("dai 0");
    depthAttachmentImages[1].SetName("dai 1");
#endif

    //before getting into the render, the layouts of the attachments need to be transitioned
    {
        EWE::CommandPool stc_cmdPool{logicalDevice, *renderQueue, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT};
        
        VkCommandBufferAllocateInfo cmdBufAllocInfo{};
        cmdBufAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmdBufAllocInfo.pNext = nullptr;
        cmdBufAllocInfo.commandBufferCount = 1;
        cmdBufAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmdBufAllocInfo.commandPool = stc_cmdPool.commandPool;
        
        VkCommandBuffer temp_stc_cmdBuf;
        EWE::EWE_VK(vkAllocateCommandBuffers, logicalDevice.device, &cmdBufAllocInfo, &temp_stc_cmdBuf);
        stc_cmdPool.allocatedBuffers++;
        
        EWE::CommandBuffer transition_stc(stc_cmdPool, temp_stc_cmdBuf);
        VkCommandBufferBeginInfo beginSTCInfo{};
        beginSTCInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginSTCInfo.pNext = nullptr;
        beginSTCInfo.pInheritanceInfo = nullptr;
        beginSTCInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        transition_stc.Begin(beginSTCInfo);

        std::vector<VkImageMemoryBarrier2> transition_barriers(4);

        uint64_t current_barrier_index = 0;

        for(auto& cai : colorAttachmentImages){
            transition_barriers[current_barrier_index].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
            transition_barriers[current_barrier_index].pNext = nullptr;
            transition_barriers[current_barrier_index].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            transition_barriers[current_barrier_index].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            transition_barriers[current_barrier_index].srcAccessMask = VK_ACCESS_2_NONE;
            transition_barriers[current_barrier_index].dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
            transition_barriers[current_barrier_index].image = cai.image;
            transition_barriers[current_barrier_index].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            transition_barriers[current_barrier_index].newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            transition_barriers[current_barrier_index].subresourceRange = EWE::ImageView::GetDefaultSubresource(cai);
            transition_barriers[current_barrier_index].srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
            transition_barriers[current_barrier_index].dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

            current_barrier_index++;
        }
        for(auto& dai : depthAttachmentImages){
            transition_barriers[current_barrier_index].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
            transition_barriers[current_barrier_index].pNext = nullptr;
            transition_barriers[current_barrier_index].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            transition_barriers[current_barrier_index].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            transition_barriers[current_barrier_index].srcAccessMask = VK_ACCESS_2_NONE;
            transition_barriers[current_barrier_index].dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            transition_barriers[current_barrier_index].image = dai.image;
            transition_barriers[current_barrier_index].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            transition_barriers[current_barrier_index].newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
            transition_barriers[current_barrier_index].subresourceRange = EWE::ImageView::GetDefaultSubresource(dai);
            transition_barriers[current_barrier_index].srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
            transition_barriers[current_barrier_index].dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT;

            current_barrier_index++;
        }

        VkDependencyInfo transition_dependency{};
        transition_dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        transition_dependency.pNext = nullptr;
        transition_dependency.dependencyFlags = 0;
        transition_dependency.bufferMemoryBarrierCount = 0;
        transition_dependency.memoryBarrierCount = 0;
        transition_dependency.imageMemoryBarrierCount = static_cast<uint32_t>(transition_barriers.size());
        transition_dependency.pImageMemoryBarriers = transition_barriers.data();

        vkCmdPipelineBarrier2(transition_stc, &transition_dependency);

        transition_stc.End();

        VkFenceCreateInfo fenceCreateInfo{};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.pNext = nullptr;
        fenceCreateInfo.flags = 0;
        EWE::Fence stc_fence{ logicalDevice, fenceCreateInfo };

        VkSubmitInfo stc_submit_info{};
        stc_submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        stc_submit_info.pNext = nullptr;
        stc_submit_info.commandBufferCount = 1;
        stc_submit_info.pCommandBuffers = &temp_stc_cmdBuf;
        stc_submit_info.signalSemaphoreCount = 0;
        stc_submit_info.waitSemaphoreCount = 0;
        stc_submit_info.pWaitDstStageMask = nullptr;

        renderQueue->Submit(1, &stc_submit_info, stc_fence);

        EWE::EWE_VK(vkWaitForFences, logicalDevice.device, 1, &stc_fence.vkFence, VK_TRUE, 5 * static_cast<uint64_t>(1.0e9));
        
        for(auto& cai : colorAttachmentImages){
            cai.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }
        for(auto& dai : depthAttachmentImages){
            dai.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        }
    }

    EWE::PerFlight<EWE::ImageView> colorAttViews{ colorAttachmentImages};
    EWE::PerFlight<EWE::ImageView> depthAttViews{ depthAttachmentImages};

    //pipeline
    auto* triangle_vert = framework.shaderFactory.GetShader("examples/common/shaders/basic.vert.spv");
    auto* triangle_frag = framework.shaderFactory.GetShader("examples/common/shaders/basic.frag.spv");

    EWE::PipeLayout triangle_layout(framework, std::initializer_list<EWE::Shader*>{ triangle_vert, triangle_frag });
    //passconfig should be using a full rendergraph setup
    EWE::PipelinePassConfig passConfig;
    passConfig.SetDefaults();
    passConfig.depthStencilInfo.depthTestEnable = VK_FALSE;
    passConfig.depthStencilInfo.depthWriteEnable = VK_FALSE;
    passConfig.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    passConfig.depthStencilInfo.stencilTestEnable = VK_FALSE;

    EWE::PipelineObjectConfig objectConfig;
    objectConfig.SetDefaults();
    objectConfig.cullMode = VK_CULL_MODE_NONE;
    objectConfig.depthClamp = false;
    objectConfig.rasterizerDiscard = false;

    std::vector<VkDynamicState> dynamicState{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    EWE::GraphicsPipeline triangle_pipeline{ logicalDevice, 0, &triangle_layout, passConfig, objectConfig, dynamicState };


    EWE::CommandRecord renderRecord{};
    renderRecord.BeginRender();
    auto* def_pipe = renderRecord.BindPipeline();
    auto* def_vp_scissor = renderRecord.SetViewportScissor();
    //auto* def_desc = cmdRecord.BindDescriptor();
    uint32_t pushIndex = renderRecord.Push();
    auto* def_draw = renderRecord.Draw();
    renderRecord.EndRender();

    EWE::RenderGraph renderGraph{logicalDevice, swapchain, *renderQueue};
    EWE::GPUTask& renderTask = renderGraph.tasks.AddElement(logicalDevice, *renderQueue, renderRecord, "render");

    def_pipe->data->pipe = triangle_pipeline.vkPipe;
    def_pipe->data->layout = triangle_pipeline.pipeLayout->vkLayout;
    def_pipe->data->bindPoint = triangle_pipeline.pipeLayout->bindPoint;

    auto& color_att = renderTask.renderTracker->compact.color_attachments.emplace_back();
    color_att.imageView[0] = &colorAttViews[0];
    color_att.imageView[1] = &colorAttViews[1];
    color_att.clearValue.color.float32[0] = 0.f;
    color_att.clearValue.color.float32[1] = 0.f;
    color_att.clearValue.color.float32[2] = 0.f;
    color_att.clearValue.color.float32[3] = 0.f;
    color_att.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_att.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    auto& depth_att = renderTask.renderTracker->compact.depth_attachment;
    depth_att.imageView[0] = &depthAttViews[0];
    depth_att.imageView[1] = &depthAttViews[1];
    depth_att.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_att.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_att.clearValue.depthStencil.depth = 0.f;
    depth_att.clearValue.depthStencil.stencil = 0; //idk what to set this to tbh

    renderTask.SetRenderInfo();

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

    struct alignas(16) TriangleVertex {
        float pos[2]; //xy
        float color[3]; //rgb, the 4th element isnt read, but i need it for alignment
    };
    for (auto& str : triangle_vert->structData) {
        if (str.name == "Vertex") {

#if EWE_DEBUG_BOOL
            printf("size comparison - %zu : %zu\n", str.size, sizeof(TriangleVertex));
            //im getting fucked by spv. its forcing 32bit alignment, even tho spirvcross says 20bit
            //assert(str.size == sizeof(TriangleVertex));
#endif
        }
    }
    EWE::Buffer vertex_buffer{framework, sizeof(TriangleVertex) * 3, 1, vmaAllocInfo, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT };
#if EWE_DEBUG_NAMING
    vertex_buffer.SetName("vertex buffer");
#endif
    {
        TriangleVertex* mappedData = reinterpret_cast<TriangleVertex*>(vertex_buffer.Map());

        mappedData[0].pos[0] = -0.5f;
        mappedData[0].pos[1] = -0.5f;

        mappedData[0].color[0] = 1.f;
        mappedData[0].color[1] = 0.f;
        mappedData[0].color[2] = 0.f;

        mappedData[1].pos[0] = 0.f;
        mappedData[1].pos[1] = 0.5f;

        mappedData[1].color[0] = 0.f;
        mappedData[1].color[1] = 1.f;
        mappedData[1].color[2] = 0.f;

        mappedData[2].pos[0] = 0.5f;
        mappedData[2].pos[1] = -0.5f;

        mappedData[2].color[0] = 0.f;
        mappedData[2].color[1] = 0.f;
        mappedData[2].color[2] = 1.f;

        vertex_buffer.Flush();
        vertex_buffer.Unmap();
    }

    {
        EWE::ResourceUsageData resourceUsageData{
            .stage = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT,
            .accessMask = VK_ACCESS_2_DESCRIPTOR_BUFFER_READ_BIT_EXT
        };

        renderTask.PushBuffer(&vertex_buffer, pushIndex, 0, resourceUsageData);

        def_draw->data->firstInstance = 0;
        def_draw->data->firstVertex = 0;
        def_draw->data->instanceCount = 1;
        def_draw->data->vertexCount = 3;
    }
    EWE::CommandRecord presentRecord{};
    auto* deferredBlit = presentRecord.Blit();

    //i think the rendergraph creates and defines the present task
    uint32_t swapchainImageIndex = 0;
    EWE::GPUTask& presentTask = renderGraph.tasks.AddElement(logicalDevice, *renderQueue, presentRecord, "present blit");


    VkSubmitInfo submitInfo{};
    submitInfo.commandBufferCount = 1;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.pSignalSemaphores = nullptr;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.waitSemaphoreCount = 1;
    VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    submitInfo.pWaitDstStageMask = &waitDstStageMask;

    //VkImageBlit imageBlit{};
    //imageBlit.srcSubresource

    VkCommandBufferBeginInfo cmdBeginInfo{};
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginInfo.pNext = nullptr;
    cmdBeginInfo.pInheritanceInfo = nullptr;
    cmdBeginInfo.flags = 0;

    deferredBlit->data->filter = VK_FILTER_LINEAR;
    VkImageBlit& blitParams = deferredBlit->data->blitParams;
    blitParams.srcOffsets[0].x = 0;
    blitParams.srcOffsets[0].y = 0;
    blitParams.srcOffsets[0].z = 0;

    blitParams.dstOffsets[0].x = 0;
    blitParams.dstOffsets[0].y = 0;
    blitParams.dstOffsets[0].z = 0;

    blitParams.srcOffsets[1].x = colorAttViews[0].image.extent.width;
    blitParams.srcOffsets[1].y = colorAttViews[0].image.extent.height;
    blitParams.srcOffsets[1].z = colorAttViews[0].image.extent.depth;

    blitParams.dstOffsets[1].x = swapchain.swapCreateInfo.imageExtent.width;
    blitParams.dstOffsets[1].y = swapchain.swapCreateInfo.imageExtent.height;
    blitParams.dstOffsets[1].z = 1;

    blitParams.srcSubresource.aspectMask = colorAttViews[0].subresource.aspectMask;
    blitParams.srcSubresource.baseArrayLayer = 0;
    blitParams.srcSubresource.layerCount = colorAttViews[0].subresource.layerCount;
    blitParams.srcSubresource.mipLevel = 0;

    blitParams.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitParams.dstSubresource.baseArrayLayer = 0;
    blitParams.dstSubresource.layerCount = 1;
    blitParams.dstSubresource.mipLevel = 0;

    //, VkCommandPoolCreateFlags createFlag
    EWE::CommandPool renderCmdPool{ logicalDevice, *renderQueue, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT};
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

    auto& taskBridge = renderGraph.bridges.AddElement(renderTask, presentTask);
    auto& prepBridge = renderGraph.bridges.AddElement(/*if only 1 task is passed in, it's on the right hand side*/renderTask);

    renderGraph.execution_order = { &prepBridge, &renderTask, &taskBridge, &presentTask };

    try {
        uint64_t totalFrames = 0;
        auto timeBegin = std::chrono::high_resolution_clock::now();
        VkDescriptorImageInfo descImg;
        uint8_t frameIndex = 0;
        std::chrono::nanoseconds elapsedTime = std::chrono::nanoseconds(0);
        constexpr auto frameDuration = std::chrono::duration<double>(1.0 / 60.0); // seconds per frame
        while (true) {
            const auto timeEnd = std::chrono::high_resolution_clock::now();
            elapsedTime += timeEnd - timeBegin;
            timeBegin = timeEnd;
            if (elapsedTime >= frameDuration) {

                glfwPollEvents();

                if (renderGraph.Acquire(frameIndex)) {
                    auto& swapPackage = swapchain.GetImagePackage();
                    EWE::Image swapImage{ logicalDevice };
                    swapImage.image = swapPackage.image;
                    swapImage.layout = VK_IMAGE_LAYOUT_UNDEFINED;
                    swapImage.owningQueue = renderQueue;//i dont know how to logically set this, but this will work
                    swapImage.mipLevels = 1;
                    swapImage.arrayLayers = 1;

                    deferredBlit->data->srcImage = colorAttachmentImages[frameIndex].image;
                    deferredBlit->data->srcLayout = colorAttachmentImages[frameIndex].layout;
                    deferredBlit->data->dstImage = swapPackage.image;
                    deferredBlit->data->dstLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

                    renderTask.UpdateFrameIndex(frameIndex);

                    presentTask.DefineBlitUsage(0, &colorAttachmentImages[frameIndex], &swapImage);

                    taskBridge.RecreateBarriers(frameIndex);
                    prepBridge.RecreateBarriers(frameIndex);

                    EWE::CommandBuffer& currentCmdBuf = commandBuffers[frameIndex];
                    //EWE::EWE_VK(vkBeginCommandBuffer, currentCmdBuf.cmdBuf, &cmdBeginInfo);
                    currentCmdBuf.Begin(cmdBeginInfo);
                    renderGraph.Execute(currentCmdBuf, frameIndex);

                    if (swapImage.layout != VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
                        //create and submit a barrier here
                        VkImageMemoryBarrier2 present_img_barrier;
                        present_img_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
                        present_img_barrier.pNext = nullptr;
                        //what was the last usage? i know what it is here, but how would I logically interpret it?
                        //a lot of the members here dont have any logical feedback
                        present_img_barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
                        present_img_barrier.dstAccessMask = VK_ACCESS_2_NONE;
                        present_img_barrier.srcQueueFamilyIndex = renderQueue->family.index;
                        present_img_barrier.dstQueueFamilyIndex = renderGraph.presentQueue.family.index;//this is the same as the renderQueue
                        present_img_barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
                        present_img_barrier.dstStageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;

                        present_img_barrier.oldLayout = swapImage.layout;
                        present_img_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

                        present_img_barrier.image = swapImage.image;
                        present_img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                        present_img_barrier.subresourceRange.baseArrayLayer = 0;
                        present_img_barrier.subresourceRange.baseMipLevel = 0;
                        present_img_barrier.subresourceRange.layerCount = 1;
                        present_img_barrier.subresourceRange.levelCount = 1;

                        VkDependencyInfo depenInfo{};
                        depenInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
                        depenInfo.pNext = nullptr;
                        depenInfo.bufferMemoryBarrierCount = 0;
                        depenInfo.memoryBarrierCount = 0;
                        depenInfo.imageMemoryBarrierCount = 1;
                        depenInfo.pImageMemoryBarriers = &present_img_barrier;
                        vkCmdPipelineBarrier2(currentCmdBuf, &depenInfo);
                    }

                    //EWE::EWE_VK(vkEndCommandBuffer, currentCmdBuf);
                    currentCmdBuf.End();
                    submitInfo.pCommandBuffers = &currentCmdBuf.cmdBuf;
                    //EWE::EWE_VK(vkQueueSubmit, *renderQueue, 1, &submitInfo, VK_NULL_HANDLE);
                    submitInfo.signalSemaphoreCount = 1;
                    submitInfo.pWaitSemaphores = &swapPackage.acquire_semaphore.vkSemaphore;
                    submitInfo.pWaitDstStageMask = &waitDstStageMask;
                    submitInfo.pSignalSemaphores = &swapPackage.present_semaphore.vkSemaphore;
                    submitInfo.waitSemaphoreCount = 1;
                    renderQueue->Submit(1, &submitInfo, swapchain.inFlightFences[frameIndex].vkFence);
                    renderGraph.Present();

                    swapchain.imageIndex = (swapchain.imageIndex + 1) % swapchain.swap_image_package.size();
                    frameIndex = (frameIndex + 1) % EWE::max_frames_in_flight;
                    totalFrames++;
                }
                else {
                    //need to recreate swapchain, thats handled internally
                    //any other Swapchain::Recreate 'callbacks' will be put here
                    blitParams.dstOffsets[1].x = swapchain.swapCreateInfo.imageExtent.width;
                    blitParams.dstOffsets[1].y = swapchain.swapCreateInfo.imageExtent.height;
                    blitParams.dstOffsets[1].z = 1;
                }
                elapsedTime = std::chrono::nanoseconds(0);
            }
        }
    }
    catch (EWE::EWEException& except) {
        framework.HandleVulkanException(except);
    }


#if EWE_DEBUG_BOOL
    printf("returning successfully\n");
#endif

    delete triangle_vert;
    delete triangle_frag;


    std::this_thread::sleep_for(std::chrono::seconds(5)); 
    return 0;
}