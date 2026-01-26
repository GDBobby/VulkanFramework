#pragma once

#include "EightWinds/VulkanHeader.h"

//#include "EightWinds/ImageView.h"

#include <EightWinds/Data/PerFlight.h>


namespace EWE{

    struct ImageView;

    struct RenderInfo {
        //i could make this an array of 4 or some arbitrary number, so that the attachments are cache local
        //idk if it matters?
        std::vector<VkRenderingAttachmentInfo> colorAttachmentInfo{};
        VkRenderingAttachmentInfo depthAttachmentInfo;

        void FixPointers(VkRenderingInfo& renderingInfo) {
            renderingInfo.pColorAttachments = colorAttachmentInfo.data();
            renderingInfo.pDepthAttachment = &depthAttachmentInfo;
            renderingInfo.pStencilAttachment = nullptr; //idk
        }
    };

    struct AttachmentInfo {
        VkAttachmentLoadOp      loadOp;
        VkAttachmentStoreOp     storeOp;
        VkClearValue            clearValue;
    };
    //temporarily just going to pretend like resolve doesnt exist
    struct SimplifiedAttachment {
        PerFlight<ImageView*> imageView{ nullptr }; //i dont know if i want ownership or view here
        AttachmentInfo info;
    };
    struct RenderInfo2 {
        VkRenderingFlags flags;
        std::vector<SimplifiedAttachment> color_attachments;
        SimplifiedAttachment depth_attachment;  //if the image is nullptr, it's not in use.

        //if resolve.size() > color_attachments.size(), then resolve.back() would be for depth
        //std::vector<Resolve> resolves{};

        void Expand(PerFlight<RenderInfo>& renderInfo, PerFlight<VkRenderingInfo>& renderingInfo) const;
        //all sizes must be uniform at the moment, i dont know if i ever want it different
        VkRect2D CalculateRenderArea() const noexcept;
    };
} //namespace EWE