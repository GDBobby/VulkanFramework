#include "EightWinds/SwapChain.h"

#include <algorithm>

namespace EWE{


        Swapchain::Swapchain(LogicalDevice& logicalDevice) noexcept
        : logicalDevice{logicalDevice},
            drawSemaphores{logicalDevice, false},
            presentSemaphores{logicalDevice, false},
            drawnFences{logicalDevice}
        {}

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

            printf("potentially need to debug this default return, idk how it behaves\n");
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