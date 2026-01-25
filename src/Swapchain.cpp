#include "EightWinds/Swapchain.h"

#include <algorithm>
#include <cassert>
#include <limits>


namespace EWE{

    VkFenceCreateInfo GetFenceCreateInfo() {
        return VkFenceCreateInfo{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
        };
    }

    Swapchain::Swapchain(LogicalDevice& logicalDevice, Window& window, Queue& presentQueue) noexcept
        : logicalDevice{logicalDevice},
        window{window},
        presentQueue{presentQueue},
        swapCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR, 
            .pNext = nullptr,
            .flags = 0,
            .surface = window.surface,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &presentQueue.family.index
        },
        activeSwapchain{VK_NULL_HANDLE},
        images{0},
        acquire_semaphores{logicalDevice, false},
        inFlightFences{logicalDevice, GetFenceCreateInfo()}
    {
        CreateSwapchain();
#if EWE_DEBUG_NAMING
        for (uint8_t i = 0; i < max_frames_in_flight; i++) {
            std::string debugName = std::string("swapchain acquire semaphore [") + std::to_string(i) + ']';
            acquire_semaphores[i].SetName(debugName.c_str());
        }
#endif
    }

    bool Swapchain::CreateSwapchain(){
        uint32_t presentCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(logicalDevice.physicalDevice.device, window.surface, &presentCount, nullptr);
        available_presentModes.resize(presentCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(logicalDevice.physicalDevice.device, window.surface, &presentCount, available_presentModes.data());

        uint32_t surfaceFormatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(logicalDevice.physicalDevice.device, window.surface, &surfaceFormatCount, nullptr);
        available_surface_formats.resize(surfaceFormatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(logicalDevice.physicalDevice.device, window.surface, &surfaceFormatCount, available_surface_formats.data());
        surface_format = Swapchain::GetSurfaceFormat(available_surface_formats);

        swapCreateInfo.imageColorSpace = surface_format.colorSpace;
        swapCreateInfo.imageFormat = surface_format.format;
        swapCreateInfo.presentMode = GetOptimalPresentMode();

        /*
        theres 2 strategies available here

        1. render directly into the swapchain image (requires VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT in imageUsage)
        2. copy the render into the swap image after finished

        if I do 1, i need to transition the image from VK_IMAGE_LAYOUT_PRESENT_SRC to color_attachment_optimal and back, per frame
        if I do 2, no layout transitions, but need an additional image

        i think im gonna do 2
        */


        VkSemaphoreCreateInfo semCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0
        };

        return RecreateSwapchain();
    }

    bool Swapchain::RecreateSwapchain(){
        if(window.screenDimensions.width == 0 || window.screenDimensions.height == 0){
            //minimized?
            return false;
        }
        VkExtent2D framebufferExtent{
            window.screenDimensions.width,
            window.screenDimensions.height
        };

        //look at vkGetPhysicalDeviceSurfaceCapabilities2KHR as well
        VkPhysicalDeviceSurfaceInfo2KHR surfaceInfo2{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR,
            .pNext = nullptr,
            //fullscreen ext would be put in the pnext
            .surface = window.surface
        };
        VkSurfaceCapabilities2KHR surfaceCapabilities2{
            .sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR,
            .pNext = nullptr
        };
        vkGetPhysicalDeviceSurfaceCapabilities2KHR(logicalDevice.physicalDevice.device, &surfaceInfo2, &surfaceCapabilities2);


        swapCreateInfo.compositeAlpha = Swapchain::GetCompositeAlpha(surfaceCapabilities2.surfaceCapabilities);
        swapCreateInfo.imageExtent = Swapchain::GetImageExtent(surfaceCapabilities2.surfaceCapabilities, framebufferExtent);

        swapCreateInfo.minImageCount = Swapchain::GetImageCount(surfaceCapabilities2.surfaceCapabilities);
        swapCreateInfo.oldSwapchain = activeSwapchain;
        swapCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapCreateInfo.preTransform = surfaceCapabilities2.surfaceCapabilities.currentTransform;

        //wait for each fence
        std::vector<VkFence> fences{};
        for(auto& fence : inFlightFences){
            fences.push_back(fence.vkFence);
        }
        //timeout is not an accepted result here
        EWE_VK(vkWaitForFences, logicalDevice.device, static_cast<uint32_t>(fences.size()), fences.data(), VK_TRUE, UINT64_MAX);

        EWE_VK(vkCreateSwapchainKHR, logicalDevice.device, &swapCreateInfo, nullptr, &activeSwapchain);

        //10 is an arbitrary number meant to be at least as large as the most images that would ever be acquired
        uint32_t swapImageCount = 0;
        EWE_VK(vkGetSwapchainImagesKHR, logicalDevice.device, activeSwapchain, &swapImageCount, nullptr);
        std::vector<VkImage> raw_images(swapImageCount);
        EWE_VK(vkGetSwapchainImagesKHR, logicalDevice.device, activeSwapchain, &swapImageCount, raw_images.data());
        images.DestroyAll();
        images.Resize(swapImageCount);
        images.ConstructAll(logicalDevice);
        for (uint8_t i = 0; i < swapImageCount; i++) {
            auto& backImage = images[i];

            backImage.image = raw_images[i];
            backImage.layout = VK_IMAGE_LAYOUT_UNDEFINED;
            backImage.owningQueue = nullptr; //i need to make sure this is handled properly
            backImage.mipLevels = 1;
            backImage.arrayLayers = 1;
            backImage.format = swapCreateInfo.imageFormat;
            backImage.type = VK_IMAGE_TYPE_2D;
            backImage.extent.width = swapCreateInfo.imageExtent.width;
            backImage.extent.height = swapCreateInfo.imageExtent.height;

#if EWE_DEBUG_NAMING
            std::string imageIndexStr = std::string("swap chain image [") + std::to_string(i) + ']';
            backImage.SetName(imageIndexStr.c_str());
#endif
        }

        present_semaphores.clear(); //these aren't getting destructed
        present_semaphores.reserve(swapImageCount);
        for (uint8_t i = 0; i < swapImageCount; i++){
            present_semaphores.push_back(Semaphore{ logicalDevice, false });
#if EWE_DEBUG_NAMING
            std::string debugName = std::string("swapchain present semaphore [") + std::to_string(i) + ']';
            present_semaphores[i].SetName(debugName.c_str());
#endif
        }

        imageIndex = 0;
        currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        return true;
    }
    bool Swapchain::RecreateSwapchain(VkPresentModeKHR desiredPresentMode){
        //maybe assert that this exists within available present modes?
        //otherwise whats the point in storing them
        swapCreateInfo.presentMode = desiredPresentMode;
        return RecreateSwapchain(); 
    }

    bool Swapchain::AcquireNextImage(uint8_t frameIndex){

        //fix this
        auto window_dimensions = window.screenDimensions;
        if(window_dimensions.width == 0 || window_dimensions.height == 0){
            //this means its minimized i think
            return false;
        }
        if((window_dimensions.width != swapCreateInfo.imageExtent.width) || (window_dimensions.height != swapCreateInfo.imageExtent.height) || wantsToRecreate){
            //resized
            if(!RecreateSwapchain()){
            //if recreating failed, return false
                return false;
            }
            wantsToRecreate = false;
        }

        //wait for 2 seconds, then timeout, which will assert or throw
        //its nanoseconds, which have 9 zeros i guess
        static constexpr uint64_t fence_timeout_v = static_cast<uint64_t>(2.0e9);
        EWE_VK(vkWaitForFences, logicalDevice.device, 1, &inFlightFences[frameIndex].vkFence, VK_TRUE, fence_timeout_v);
        EWE_VK(vkResetFences, logicalDevice.device, 1, &inFlightFences[frameIndex].vkFence);

        uint32_t image_index;

        /*
            synchronization scope - currently this is all within one thread, which would be the graphics thread. maybe the rendergraph changes that

            vkAcquireNextImageKHR
            vkAcquireNextImage2KHR
            vkQueuePresentKHR
            vkGetSwapchainImagesKHR
            vkCreateSwapchainKHR
            vkDestroySwapchainKHR
        */
       
        //printf("lock queue mutex here\n");
        VkResult acquireResult = vkAcquireNextImageKHR(logicalDevice.device, activeSwapchain, UINT64_MAX, acquire_semaphores[frameIndex].vkSemaphore, VK_NULL_HANDLE, &image_index);
        //printf("unlock queue mutex\n");
        
        switch(acquireResult){
            case VK_SUCCESS: break; //dont do anything
            case VK_SUBOPTIMAL_KHR: break; //dont do anythign? need to look into this
            case VK_ERROR_OUT_OF_DATE_KHR: RecreateSwapchain(); return false;
            default: break;
        }
        EWE_VK_RESULT(acquireResult); //throws or asserts

#if EWE_DEBUG_BOOL
        //printf("acquired image index : %u\n", image_index);
#endif

        imageIndex = image_index; //object index equal to local index
        return true;
    }

    VkPresentModeKHR Swapchain::GetOptimalPresentMode() const {
            
        static constexpr auto desired_v = std::array{VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_RELAXED_KHR};
        for (auto const desired : desired_v) {
            if (std::ranges::find(available_presentModes, desired) != available_presentModes.end()) { return desired; }
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D Swapchain::GetImageExtent(VkSurfaceCapabilitiesKHR const& caps, VkExtent2D framebuffer) noexcept {
        constexpr auto limitless_v = std::numeric_limits<std::uint32_t>::max();
        if (caps.currentExtent.width < limitless_v && caps.currentExtent.height < limitless_v) { return caps.currentExtent; }
        auto const x = std::clamp(framebuffer.width, caps.minImageExtent.width, caps.maxImageExtent.width);
        auto const y = std::clamp(framebuffer.height, caps.minImageExtent.height, caps.maxImageExtent.height);
        return VkExtent2D{.width = x, .height = y};
    }
    
    uint32_t Swapchain::GetImageCount(VkSurfaceCapabilitiesKHR const& caps) noexcept {
        if (caps.maxImageCount < caps.minImageCount) { return std::max(3u, caps.minImageCount + 1); }
        return std::clamp(3u, caps.minImageCount + 1, caps.maxImageCount);
    }

    VkSurfaceFormatKHR Swapchain::GetSurfaceFormat(std::span<VkSurfaceFormatKHR const> supported) noexcept {
        constexpr auto srgb_formats_v = std::array{VK_FORMAT_R8G8B8A8_SRGB, VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_A8B8G8R8_SRGB_PACK32};

        for (auto const srgb_format : srgb_formats_v) {
            auto const it = std::ranges::find_if(supported, [srgb_format](VkSurfaceFormatKHR const& format) {
                return format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && format.format == srgb_format;
                
            });
            if (it != supported.end()) { return *it; }
        }

#if EWE_DEBUG_BOOL
        printf("potentially need to debug this default return, idk how it behaves\n");
#endif
        return VkSurfaceFormatKHR{};
    }

    VkCompositeAlphaFlagBitsKHR Swapchain::GetCompositeAlpha(VkSurfaceCapabilitiesKHR const& caps) noexcept {
        if (caps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) { return VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; }
        if (caps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) { return VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR; }
        if (caps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR) { return VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR; }
        // according to the spec, at least one bit must be set
        return VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
    }

    Image& Swapchain::GetCurrentImage() {
        return images[imageIndex];
    }
}