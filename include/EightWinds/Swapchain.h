#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/LogicalDevice.h"
#include "EightWinds/Window.h"
#include "EightWinds/Backend/Semaphore.h"
#include "EightWinds/Backend/Fence.h"
#include "EightWinds/Image.h"
#include "EightWinds/ImageView.h"

#include "EightWinds/CommandBuffer.h"

#include "EightWinds/Data/RuntimeArray.h"
#include "EightWinds/Data/PerFlight.h"

#include "EightWinds/Data/HeapBlock.h"


#include <array>
#include <span>
#include <vector>

//https://docs.vulkan.org/refpages/latest/refpages/source/VK_NV_low_latency2.html
//https://docs.vulkan.org/refpages/latest/refpages/source/vkLatencySleepNV.html


//https://github.com/karnkaul/LittleEngineVk/blob/levk-23/lib/levk/src/render_device.cpp
//starting with a straight copy of karnage's swapchain

/*
* small notes on the changes from karnage's implementation

i dont think i want the swapchain to own the command buffer
it would imply that anything that uses that command buffer would be using the swapchain,
and that the command buffer begins and ends with the swap chain
thats fine in smaller apps, but if i do pre-compute or post-compute, or whatever, it's less applicable
*/

namespace EWE{

    struct Swapchain{
        LogicalDevice& logicalDevice;
        Window& window;
        Queue& presentQueue;

        [[nodiscard]] explicit Swapchain(LogicalDevice& logicalDevice, Window& window, Queue& presentQueue) noexcept;

        std::vector<VkPresentModeKHR> available_presentModes;
        std::vector<VkSurfaceFormatKHR> available_surface_formats;
        VkSurfaceFormatKHR surface_format;
        VkSwapchainCreateInfoKHR swapCreateInfo;//i dont nromaly keep these, ill have to come back to this
        VkSwapchainKHR activeSwapchain;

        uint32_t imageIndex = 0;
        HeapBlock<Image> images;
        HeapBlock<ImageView> imageViews;

        PerFlight<Semaphore> acquire_semaphores;
        std::vector<Semaphore> present_semaphores;

        inline Semaphore& GetAcquireSemaphore(uint8_t frameIndex) {
            return acquire_semaphores[frameIndex];
        }
        inline Semaphore& GetCurrentPresentSemaphore() {
            return present_semaphores[imageIndex];
        }

        VkImageLayout currentLayout;
        
        //WSI (windows system interface or something) doesnt work with timeline semaphore
        PerFlight<Fence> inFlightFences;

        bool wantsToRecreate = false;

        bool CreateSwapchain();
        bool RecreateSwapchain();
        bool RecreateSwapchain(VkPresentModeKHR desiredPresentMode);
        bool AcquireNextImage(uint8_t frameIndex);

        [[nodiscard]] Image& GetCurrentImage();
        [[nodiscard]] ImageView& GetCurrentImageView();

        [[nodiscard]] VkPresentModeKHR GetOptimalPresentMode() const;
        
        [[nodiscard]] static VkSurfaceFormatKHR GetSurfaceFormat(std::span<VkSurfaceFormatKHR const> supported) noexcept;
        [[nodiscard]] static VkCompositeAlphaFlagBitsKHR GetCompositeAlpha(VkSurfaceCapabilitiesKHR const& caps) noexcept;
        [[nodiscard]] static VkExtent2D GetImageExtent(VkSurfaceCapabilitiesKHR const& caps, VkExtent2D framebuffer) noexcept;

        [[nodiscard]] static uint32_t GetImageCount(VkSurfaceCapabilitiesKHR const& caps) noexcept;
    };
}