#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/LogicalDevice.h"
#include "EightWinds/Window.h"
#include "EightWinds/PerFlight.h"
#include "EightWinds/Backend/Semaphore.h"
#include "EightWinds/Backend/Fence.h"

#include "EightWinds/CommandBuffer.h"

#include <array>
#include <span>
#include <vector>

//https://github.com/karnkaul/LittleEngineVk/blob/levk-23/lib/levk/src/render_device.cpp
//starting with a straight copy of karnage's swapchain

/*
* small notes on the changes from karnage's implemenation

i dont think i want the swapchain to own the command buffer
it would imply that anything that uses that command buffer would be using the swapchain,
and that the command buffer begins and ends with the swap chain
thats fine in smaller apps, but if i do pre-compute or post-compute, or whatever, it's less applicable
*/

namespace EWE{

    struct Swapchain{
        LogicalDevice& logicalDevice;

        [[nodiscard]] explicit Swapchain(LogicalDevice& logicalDevice) noexcept;

        std::vector<VkPresentModeKHR> presentModes{};
        VkSwapchainCreateInfoKHR swapCreateInfo;//i dont nromaly keep these, ill have to come back to this
        VkSwapchainKHR swapchain;

        uint32_t imageIndex;

        std::vector<VkImage> images;
        std::vector<VkImageView> imageViews;
        
        //most likely, sync will be controlled by the rendergraph, and absorbed from here
        //but until i get that working, having them here will be helpful
        //also, the WSI (windows system interface or something) doesnt work with timeline semaphore
        PerFlight<Semaphore> drawSemaphores;
        PerFlight<Semaphore> presentSemaphores;
        PerFlight<Fence> drawnFences; 


        //im assuming htis gets absorbed by the render graph
        /*
        VkResult Present(std::span<Semaphore> semaphores) const{
            VkPresentInfoKHR presentInfo{};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            presentInfo.pNext = nullptr;
            presentInfo.waitSemaphoreCount = static_cast<uint32_t>(semaphores.size());
            presentInfo.swapchainCount = 1;
            
            std::unique_lock<std::mutex> queueLock{VK::Object->queueMutex[Queue::graphics]};
            return vkQueuePresentKHR(VK::Object->queues[Queue::present], &presentInfo);
        }
        */

        [[nodiscard]] VkPresentModeKHR GetOptimalPresentMode() const {
            
            static constexpr auto desired_v = std::array{VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_RELAXED_KHR};
            for (auto const desired : desired_v) {
                if (std::ranges::find(presentModes, desired) != presentModes.end()) { return desired; }
            }
            return VK_PRESENT_MODE_FIFO_KHR;
        }
        
        [[nodiscard]] static VkSurfaceFormatKHR GetSurfaceFormat(std::span<VkSurfaceFormatKHR const> supported) noexcept;
        [[nodiscard]] static VkCompositeAlphaFlagBitsKHR GetCompositeAlpha(VkSurfaceCapabilitiesKHR const& caps) noexcept;
        [[nodiscard]] static VkExtent2D GetImageExtent(VkSurfaceCapabilitiesKHR const& caps, VkExtent2D framebuffer) noexcept;

        [[nodiscard]] static uint32_t GetImageCount(VkSurfaceCapabilitiesKHR const& caps) noexcept;
    };
}