#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/Window.h"
#include "EightWinds/PerFlight.h"
#include "EightWinds/Semaphore.h"
#include "EightWinds/Fence.h"

#include "EightWinds/CommandBuffer.h"

#include <array>

//https://github.com/karnkaul/LittleEngineVk/blob/levk-23/lib/levk/src/render_device.cpp
//starting with a straight copy of karnage's swapchain

//small notes on the changes from karnage's implemenation
//i dont think i want the swapchain to own the command buffer
//it would imply that anything that uses that command buffer would be using the swapchain,
//and that the command buffer begins and ends with the swap chain
//thats fine in smaller apps, but if i do pre-compute or post-compute, or whatever, it's less applicable


namespace EWE{

    struct SwapChain{
        LogicalDevice& logicalDevice;

        std::vector<VkPresentModeKHR> presentModes{};
        VkSwapchainCreateInfoKHR swapCreateInfo;//i dont nromaly keep these, ill have to come back to this
        VkSwapchainKHR swapchain;

        uint32_t imageIndex;

        std::vector<VkImage> images;
        std::vector<VkImageView> imageViews;
        
        //most likely, sync will be controlled by the rendergraph, and absorbed from here
        //but until i get that working, having them here will be helpful
        PerFlight<Semaphore> drawSemaphores;
        PerFlight<Semaphore> presentSemaphore;
        PerFlight<Fence> drawnFence; 


        //im assuming htis gets absorbed by the render graph
        /*
        VkResult Present(std::span<Semaphore> semaphores) const{
            VkPresentInfoKHR presentInfo{};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            presentInfo.pNext = nullptr;
            presentInfo.waitSemaphoreCount = static_cast<uint32_t>(semaphores.size());
            presentInfo.swapchainCount = 1;
            
            std::unique_lock<std::mutex> queueLock{VK::Object->queueMutex[Queue::graphics]};
            eturn vkQueuePresentKHR(VK::Object->queues[Queue::present], &presentInfo);
        }
        */

        [[nodiscard]] VkPresentModeKHR GetOptimalPresentMode() const {
            
            static constexpr auto desired_v = std::array{VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_RELAXED_KHR};
            for (auto const desired : desired_v) {
                if (std::ranges::find(presentModes, desired) != presentModes.end()) { return desired; }
            }
            return VK_PRESENT_MODE_FIFO_KHR;
        }
        
        [[nodiscard]] static constexpr VkSurfaceFormatKHR GetSurfaceFormat(std::span<VkSurfaceFormatKHR const> supported) {
            constexpr auto srgb_formats_v = std::array{VK_FORMAT_R8G8B8A8_SRGB, VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_A8B8G8R8_SRGB_PACK32};

            for (auto const srgb_format : srgb_formats_v) {
                auto const it = std::ranges::find_if(supported, [srgb_format](VkSurfaceFormatKHR const& format) {
                    return format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && format.format == srgb_format;
                    
                });
                if (it != supported.end()) { return *it; }
            }

            printf("potentially need to debug this, idk how it behaves\n");
            return VkSurfaceFormatKHR{};
        }
        [[nodiscard]] static constexpr auto GetCompositeAlpha(VkSurfaceCapabilitiesKHR const& caps) {
            if (caps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) { return VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; }
            if (caps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) { return VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR; }
            if (caps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR) { return VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR; }
            // according to the spec, at least one bit must be set
            return VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
        }
        [[nodiscard]] static VkExtent2D GetImageExtent(VkSurfaceCapabilitiesKHR const& caps, VkExtent2D framebuffer) {
            static constexpr auto limitless_v = std::numeric_limits<std::uint32_t>::max();
            if (caps.currentExtent.width < limitless_v && caps.currentExtent.height < limitless_v) { return caps.currentExtent; }
            auto const x = std::clamp(framebuffer.width, caps.minImageExtent.width, caps.maxImageExtent.width);
            auto const y = std::clamp(framebuffer.height, caps.minImageExtent.height, caps.maxImageExtent.height);
            return VkExtent2D{x, y};
        }

        [[nodiscard]] static constexpr uint32_t GetImageCount(VkSurfaceCapabilitiesKHR const& caps) {
            if (caps.maxImageCount < caps.minImageCount) { return std::max(3u, caps.minImageCount + 1); }
            return std::clamp(3u, caps.minImageCount + 1, caps.maxImageCount);
        }
    };
}