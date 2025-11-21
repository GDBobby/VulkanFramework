#pragma once

#include "EightWinds/VulkanHeader.h"

//equivalent a renderpass subpass?

namespace EWE{
    struct GPUTask{
        
    };

    struct RenderTask{
        VkImage colorImage;
        VkImageView colorAttachmentView;
        VkFormat colorFormat;
        bool ownsColorImage; //if true, destroy on deconstruction

        VkImage depthStencilImage;
        VkImageView depthStencilView;
        VkFormat depthStencilFormat;
        bool ownsDepthImage; //if true, destroy on deconstruction

        VkImage resolveImage;
        VkImageView resolveView;
        bool ownsResolveImage; //if true, destroy on deconstruction

        VkExtent2D renderExtent;
        VkSampleCountFlagBits samples; //multi-sample-anti-aliasing
    };

    class ScopedTask {
    public:
        ScopedTask(VkCommandBuffer cmdBuf, GPUTask const& step)
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
        VkCommandBuffer m_cmdBuf;
        VkRenderingAttachmentInfo m_colorAttachmentInfo{};
        VkRenderingAttachmentInfo m_depthAttachmentInfo{};
        VkRenderingInfo m_renderingInfo{};
    };
}