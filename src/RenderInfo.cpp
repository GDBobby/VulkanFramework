#include "EightWinds/Backend/RenderInfo.h"

#include "EightWinds/ImageView.h"

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

    VkRect2D RenderInfo2::CalculateRenderArea() const noexcept {
        VkRect2D ret{};
        ret.offset.x = 0;
        ret.offset.y = 0;
        //if we're not enforcing uniform size, render area will be equal to the smallest size here
        ret.extent.width = color_attachments[0].imageView[0]->image.extent.width;
        ret.extent.height = color_attachments[0].imageView[0]->image.extent.height;
#if EWE_DEBUG_BOOL
        for (std::size_t i = 1; i < color_attachments.size(); i++) {
            assert(color_attachments[i].imageView[0]->image.extent.width == ret.extent.width);
            assert(color_attachments[i].imageView[0]->image.extent.height == ret.extent.height);
        }
        if(depth_attachment.imageView[0] != nullptr){
            assert(depth_attachment.imageView[0]->image.extent.width == ret.extent.width);
            assert(depth_attachment.imageView[0]->image.extent.height = ret.extent.height);
        }
#endif
        return ret;
    }

    void RenderInfo2::Expand(RenderInfo* out) const {
        RenderInfo& ret = *out;
        ret.renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        ret.renderingInfo.pNext = nullptr;

        assert(ret.colorAttachmentInfo.size() == 0);

        for (auto const& att : color_attachments) {
            auto& backAtt = ret.colorAttachmentInfo.emplace_back();
            backAtt.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            backAtt.pNext = nullptr;

            backAtt.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            backAtt.resolveImageView = VK_NULL_HANDLE;
            backAtt.resolveMode = VK_RESOLVE_MODE_NONE;

            backAtt.clearValue = att.info.clearValue;
            backAtt.loadOp = att.info.loadOp;
            backAtt.storeOp = att.info.storeOp;

            backAtt.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }

        ret.depthAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        ret.depthAttachmentInfo.pNext = nullptr;

        if (depth_attachment.imageView[0] == nullptr) {
            ret.depthAttachmentInfo.imageView = VK_NULL_HANDLE;
        }

        ret.depthAttachmentInfo.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        ret.depthAttachmentInfo.resolveImageView = VK_NULL_HANDLE;
        ret.depthAttachmentInfo.resolveMode = VK_RESOLVE_MODE_NONE;

        ret.depthAttachmentInfo.clearValue = depth_attachment.info.clearValue;
        ret.depthAttachmentInfo.loadOp = depth_attachment.info.loadOp;
        ret.depthAttachmentInfo.storeOp = depth_attachment.info.storeOp;
        ret.depthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;


        ret.renderingInfo.colorAttachmentCount = static_cast<uint32_t>(ret.colorAttachmentInfo.size());
        ret.renderingInfo.flags = flags;
        //layerCount is the number of layers rendered to in each attachment when viewMask is 0.
        ret.renderingInfo.layerCount = 1;
        ret.renderingInfo.pColorAttachments = ret.colorAttachmentInfo.data();
        ret.renderingInfo.pDepthAttachment = &ret.depthAttachmentInfo;
        ret.renderingInfo.renderArea = CalculateRenderArea();
    }
    RenderInfo RenderInfo2::Expand() const {

        RenderInfo ret{};
        Expand(&ret);
        return ret;
    }

    void RenderInfo2::Update(RenderInfo* out, uint8_t frameIndex) const {
        
        assert(out->colorAttachmentInfo.size() == color_attachments.size());
        for(std::size_t color_att_index = 0; color_att_index < color_attachments.size(); color_att_index++){
            out->colorAttachmentInfo[color_att_index].imageView = color_attachments[color_att_index].imageView[frameIndex]->view;
        }
        if(depth_attachment.imageView[frameIndex] != nullptr){
            out->depthAttachmentInfo.imageView = depth_attachment.imageView[frameIndex]->view;
        }
        else {
            out->renderingInfo.pDepthAttachment = nullptr;
        }
    }

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