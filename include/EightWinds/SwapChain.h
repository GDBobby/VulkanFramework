#include "EightWinds/VulkanHeader.h"
#include "EightWinds/Window.h"

namespace EWE{

    struct SwapChain{


        std::vector<vkImageView> imageViews{};
        vkFormat imageFormats{};
        vkFormat depthFormats{};
        vkExtent2D extent;
    };
}