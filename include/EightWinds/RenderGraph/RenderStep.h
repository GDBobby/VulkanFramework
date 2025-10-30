#pragma once

#include "EightWinds/VulkanHeader.h"

//equivalent a renderpass subpass?

namespace EWE{
    struct RenderStep{
        vk::Image colorImage;
        vk::ImageView colorAttachmentView;
        vk::Format colorFormat;
        bool ownsColorImage; //if true, destroy on deconstruction

        vk::Image depthStencilImage;
        vk::ImageView depthStencilView;
        vk::Format depthStencilFormat;
        bool ownsDepthImage; //if true, destroy on deconstruction

        vk::Image resolveImage;
        vk::ImageView resolveView;
        bool ownsResolveImage; //if true, destroy on deconstruction

        vk::Extent2D renderExtent;
        vk::SampleCountFlagBits samples; //multi-sample-anti-aliasing
    };

    class ScopedRendering {
    public:
        ScopedRendering(vk::CommandBuffer cmdBuf, RenderStep const& step)
            : m_cmdBuf(cmdBuf)
        {
            m_colorAttachmentInfo = vk::RenderingAttachmentInfo{
                .imageView = step.colorAttachmentView,
                .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
                .loadOp = vk::AttachmentLoadOp::eClear,
                .storeOp = vk::AttachmentStoreOp::eStore,
                .clearValue = vk::ClearValue(vk::ClearColorValue(std::array<float,4>{0.f, 0.f, 0.f, 1.f}))
            };

            if (step.resolveView) {
                m_colorAttachmentInfo.resolveMode = vk::ResolveModeFlagBits::eAverage;
                m_colorAttachmentInfo.resolveImageView = step.resolveView;
                m_colorAttachmentInfo.resolveImageLayout = vk::ImageLayout::eColorAttachmentOptimal;
            }

            if (step.depthStencilView) {
                m_depthAttachmentInfo = vk::RenderingAttachmentInfo{
                    .imageView = step.depthStencilView,
                    .imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
                    .loadOp = vk::AttachmentLoadOp::eLoad,
                    .storeOp = vk::AttachmentStoreOp::eStore,
                    .clearValue = vk::ClearValue(vk::ClearDepthStencilValue{1.0f, 0})
                };
            }

            m_renderingInfo = vk::RenderingInfo{
                .renderArea = vk::Rect2D({0, 0}, step.extent),
                .layerCount = 1,
                .colorAttachmentCount = 1,
                .pColorAttachments = &m_colorAttachmentInfo,
                .pDepthAttachment = step.depthStencilView ? &m_depthAttachmentInfo : nullptr
            };

            m_cmdBuf.beginRendering(m_renderingInfo);
        }

        ScopedRendering(RenderStep const& step){

        }

        ~ScopedRendering() {
            m_cmdBuf.endRendering();
        }

    private:
        vk::CommandBuffer m_cmdBuf;
        vk::RenderingAttachmentInfo m_colorAttachmentInfo{};
        vk::RenderingAttachmentInfo m_depthAttachmentInfo{};
        vk::RenderingInfo m_renderingInfo{};
    };
}