#include "EightWinds/VulkanHeader.h"
#include "EightWinds/Window.h"

namespace EWE{

    struct SwapChain{


        std::vector<vk::ImageView> imageViews{};
        vk::Format imageFormats{};
        vk::Format depthFormats{};
        vk::Extent2D extent;
    };
}