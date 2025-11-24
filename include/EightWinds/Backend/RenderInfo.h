#pragma once

#include "EightWinds/VulkanHeader.h"


namespace EWE{
    struct RenderInfo {
        VkRenderingAttachmentInfo colorAttachmentInfo;
        VkRenderingAttachmentInfo depthAttachmentInfo;
        VkRenderingInfo renderingInfo{};
    };
} //namespace EWE