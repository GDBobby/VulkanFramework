#include "EightWinds/Swapchain.h"

#include <algorithm>
#include <cassert>
#include <limits>


namespace EWE{

    VkFenceCreateInfo GetFenceCreateInfo() {
        VkFenceCreateInfo fenceCreateInfo{};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.pNext = nullptr;
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        return fenceCreateInfo;
    }

    Swapchain::Swapchain(LogicalDevice& logicalDevice, Window& window, Queue& presentQueue) noexcept
        : logicalDevice{logicalDevice},
        window{window},
        presentQueue{presentQueue},
        activeSwapchain{VK_NULL_HANDLE},
        inFlightFences{logicalDevice, GetFenceCreateInfo()},
        swapCreateInfo{}
    {
        swapCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapCreateInfo.pNext = nullptr;
        CreateSwapchain();
    }

    bool Swapchain::CreateSwapchain(){
        uint32_t presentCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(logicalDevice.physicalDevice.device, window.surface, &presentCount, nullptr);
        presentModes.resize(presentCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(logicalDevice.physicalDevice.device, window.surface, &presentCount, presentModes.data());

        uint32_t surfaceFormatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(logicalDevice.physicalDevice.device, window.surface, &surfaceFormatCount, nullptr);
        std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(logicalDevice.physicalDevice.device, window.surface, &surfaceFormatCount, surfaceFormats.data());
        const VkSurfaceFormatKHR surfaceFormat = Swapchain::GetSurfaceFormat(surfaceFormats);

        swapCreateInfo.imageFormat = surfaceFormat.format;
        swapCreateInfo.surface = window.surface;
        swapCreateInfo.presentMode = GetOptimalPresentMode();
        swapCreateInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        /*
        theres 2 strategies available here

        1. render directly into the swapchain image (requires VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT in imageUsage)
        2. copy the render into the swap image after finished

        if I do 1, i need to transition the image from VK_IMAGE_LAYOUT_PRESENT_SRC to color_attachment_optimal and back, per frame
        if I do 2, no layout transitions, but need an additional image

        i think im gonna do 2
        */

        swapCreateInfo.queueFamilyIndexCount = 1;
        swapCreateInfo.pQueueFamilyIndices = &presentQueue.family.index;
        swapCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
        swapCreateInfo.imageArrayLayers = 1;

        VkSemaphoreCreateInfo semCreateInfo{};
        semCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semCreateInfo.pNext = nullptr;
        semCreateInfo.flags = 0;

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
        VkPhysicalDeviceSurfaceInfo2KHR surfaceInfo2;
        surfaceInfo2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR;
        surfaceInfo2.pNext = nullptr;
        //fullscreen ext would be put in the pnext
        surfaceInfo2.surface = window.surface;
        VkSurfaceCapabilities2KHR surfaceCapabilities2{};
        surfaceCapabilities2.sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR;
        surfaceCapabilities2.pNext = nullptr;
        vkGetPhysicalDeviceSurfaceCapabilities2KHR(logicalDevice.physicalDevice.device, &surfaceInfo2, &surfaceCapabilities2);


        swapCreateInfo.flags = 0;
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
        std::vector<VkImage> tempImageBuffer(swapImageCount);
        EWE_VK(vkGetSwapchainImagesKHR, logicalDevice.device, activeSwapchain, &swapImageCount, tempImageBuffer.data());


/*
        for (auto& old : swap_image_package) {
            logicalDevice.GarbageDisposal.toss<VkDestroySemaphore>(old.acquire_semaphore, swap_image_package.size() + 1);
            logicalDevice.GarbageDisposal.toss(old.present_semaphore, swap_image_package.size() + 1);
        }
        swap_image_package.clear();
        for (uint32_t i = 0; i < swapImageCount; i++) {
            swap_image_package.push_back(
                SwapImage{
                    .image = tempImageBuffer.image,
                    .acquire_semaphore = 
                }
            );
        }
*/
        assert(swapImageCount >= swap_image_package.size() && "this is temporary, i need to setup a reduction in size");
        
        const std::size_t previous_swap_size = swap_image_package.size();
        for(uint32_t i = 0; i < previous_swap_size; i++){
            swap_image_package[i].image = tempImageBuffer[i];
        }

        for (uint32_t i = previous_swap_size; i < swapImageCount; i++) {
            swap_image_package.push_back(
                SwapImage{
                    .image = tempImageBuffer[i],
                    .present_semaphore = Semaphore{logicalDevice, false},
                    .acquire_semaphore = Semaphore{logicalDevice, false}
                }
            );
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
        if((window_dimensions.width != swapCreateInfo.imageExtent.width) || (window_dimensions.height != swapCreateInfo.imageExtent.height)){
            //resized
            if(!RecreateSwapchain()){
            //if recreating failed, return false
                return false;
            }
        }

        //wait for 2 seconds, then timeout, which will assert or throw
        //its nanoseconds, which have 9 zeros i guess
        static constexpr uint64_t fence_timeout_v = static_cast<uint64_t>(2.0e9);
        EWE_VK(vkWaitForFences, logicalDevice.device, 1, &inFlightFences[frameIndex].vkFence, VK_TRUE, fence_timeout_v);
        EWE_VK(vkResetFences, logicalDevice.device, 1, &inFlightFences[frameIndex].vkFence);
        //fence.Wait(fence_timeout_v);
        //fence.Reset();

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
        VkResult acquireResult = vkAcquireNextImageKHR(logicalDevice.device, activeSwapchain, UINT64_MAX, swap_image_package[imageIndex].acquire_semaphore.vkSemaphore, VK_NULL_HANDLE, &image_index);
        //printf("unlock queue mutex\n");

        //im assuming that it always matches. if it does not, i need to restrategize how the semaphores and images are associated
        assert(imageIndex == image_index);
        
        switch(acquireResult){
            case VK_SUCCESS: break; //dont do anything
            case VK_SUBOPTIMAL_KHR: break; //dont do anythign? need to look into this
            case VK_ERROR_OUT_OF_DATE_KHR: RecreateSwapchain(); return false;
            default: break;
        }
        EWE_VK_RESULT(acquireResult); //throws or asserts

        imageIndex = image_index; //object index equal to local index
        return true;
    }

    VkPresentModeKHR Swapchain::GetOptimalPresentMode() const {
            
            static constexpr auto desired_v = std::array{VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_RELAXED_KHR};
            for (auto const desired : desired_v) {
                if (std::ranges::find(presentModes, desired) != presentModes.end()) { return desired; }
            }
            return VK_PRESENT_MODE_FIFO_KHR;
        }

    VkExtent2D Swapchain::GetImageExtent(VkSurfaceCapabilitiesKHR const& caps, VkExtent2D framebuffer) noexcept {
        constexpr auto limitless_v = std::numeric_limits<std::uint32_t>::max();
        if (caps.currentExtent.width < limitless_v && caps.currentExtent.height < limitless_v) { return caps.currentExtent; }
        auto const x = std::clamp(framebuffer.width, caps.minImageExtent.width, caps.maxImageExtent.width);
        auto const y = std::clamp(framebuffer.height, caps.minImageExtent.height, caps.maxImageExtent.height);
        return VkExtent2D{x, y};
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
}