#include "EightWinds/Backend/RenderInfo.h"

#include <cassert>

/*
VUID-VkRenderingAttachmentInfo-pNext-11752
If the pNext chain includes a VkRenderingAttachmentFlagsInfoKHR structure, and flags includes VK_RENDERING_ATTACHMENT_RESOLVE_SKIP_TRANSFER_FUNCTION_BIT_KHR or VK_RENDERING_ATTACHMENT_RESOLVE_ENABLE_TRANSFER_FUNCTION_BIT_KHR, imageView must have a format using sRGB encoding

VUID-VkRenderingAttachmentInfo-pNext-11753
If the pNext chain includes a VkRenderingAttachmentFlagsInfoKHR structure, and flags includes VK_RENDERING_ATTACHMENT_RESOLVE_SKIP_TRANSFER_FUNCTION_BIT_KHR or VK_RENDERING_ATTACHMENT_RESOLVE_ENABLE_TRANSFER_FUNCTION_BIT_KHR, resolveMode must be equal to VK_RESOLVE_MODE_AVERAGE_BIT

VUID-VkRenderingAttachmentInfo-pNext-11754
If the pNext chain includes a VkRenderingAttachmentFlagsInfoKHR structure, and flags includes VK_RENDERING_ATTACHMENT_INPUT_ATTACHMENT_FEEDBACK_BIT_KHR, imageView must have an image that was created with the VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT usage flag set

Each pNext member of any structure (including this one) in the pNext chain must be either NULL or a pointer to a valid instance of VkAttachmentFeedbackLoopInfoEXT or VkRenderingAttachmentFlagsInfoKHR

*/

namespace EWE{

    /*
    i don't really like how much this constructor assumes, and how difficult it's going to be to customize.
    im tempted to just not use a constructor, and force the user to 

    RenderInfo::RenderInfo(VkExtent2D renderExtent, VkFormat colorFormat, VkFormat depthFormat, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT){

        VkImageCreateInfo imageCreateInfo{};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.extent = { renderExtent.width, renderExtent.height, 1 };
        imageCreateInfo.mipLevels = 1;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.samples = samples;
        imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;


        //create color and depth
        vkCreateImage(logicalDevice.device, &imageCreateInfo, nullptr, &colorAttachment.image);

        colorAttachmentInfo = VkRenderingAttachmentInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .pNext = nullptr,
            .imageView = colorAttachment.view,
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .resolveMode = resolveAttachment.mode,
            .resolveImageView = resolveAttachment.view,
            .resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = { .color = {{0.f, 0.f, 0.f, 1.f}} }
        };

        depthAttachmentInfo = VkRenderingAttachmentInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .pNext = nullptr,
            .imageView = depthAttachment.view,
            .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .resolveMode = VK_RESOLVE_MODE_NONE,
            .resolveImageView = VK_NULL_HANDLE,
            .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .clearValue = { .depthStencil = {1.0f, 0} }
        };

        VkRenderingInfo renderingInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .pNext = nullptr,
            .flags = 0,
            .renderArea = {
                .offset = {0, 0},
                .extent = renderExtent
            },
            .layerCount = 1,
            .viewMask = 0, // No multiview
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachmentInfo,
            .pDepthAttachment = &depthAttachmentInfo,  // or nullptr
            .pStencilAttachment = &depthAttachmentInfo // same for typical combined depth/stencil
        };
    }
        */
}// namespace EWE