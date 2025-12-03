#pragma once

#include "EightWinds/VulkanHeader.h"

//#include "EightWinds/ImageView.h"

#include <array>


namespace EWE{

    struct ImageView;

    struct RenderInfo {
        //i could make this an array of 4 or some arbitrary number, so that the attachments are cache local
        //idk if it matters?
        std::vector<VkRenderingAttachmentInfo> colorAttachmentInfo;
        VkRenderingAttachmentInfo depthAttachmentInfo;
        VkRenderingInfo renderingInfo{};

        void FixPointers() {
            renderingInfo.pColorAttachments = colorAttachmentInfo.data();
            renderingInfo.pDepthAttachment = &depthAttachmentInfo;
            renderingInfo.pStencilAttachment = nullptr; //idk
        }
    };

    //temporarily just going to pretend like resolve doesnt exist
    struct SimplifiedAttachment {
        ImageView*              imageView; //i dont know if i want ownership or view here
        VkAttachmentLoadOp      loadOp;
        VkAttachmentStoreOp     storeOp;
        VkClearValue            clearValue;
    };
    struct RenderInfo2 {
        VkRenderingFlags flags;
        std::vector<SimplifiedAttachment> color_attachments;
        SimplifiedAttachment depth_attachment;  //if the image is nullptr, it's not in use.

        //if resolve.size() > color_attachments.size(), then resolve.back() would be for depth
        //std::vector<Resolve> resolves{};

        //call RenderInfo::FixPointers after constructing RenderInfo with Expand()
        RenderInfo Expand() const;
        //Expand(RenderInfo* out) fixes the pointers in place since it wont potentially be moved on return
        void Expand(RenderInfo* out) const;
        //all sizes must be uniform at the moment, i dont know if i ever want it different
        VkRect2D CalculateRenderArea() const;
    };
} //namespace EWE