#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/RenderGraph/GPUTask.h"



namespace EWE{
    struct PresentBridge{
        LogicalDevice& logicalDevice;
        Queue& presentQueue;
        const std::string name;

        [[nodiscard]] explicit PresentBridge(LogicalDevice& logicalDevice, Queue& presentQueue) noexcept;

        PresentBridge(PresentBridge const& copySrc) = delete;
        PresentBridge(PresentBridge&& moveSrc) noexcept = delete;

        PresentBridge& operator=(PresentBridge const& copySrc) = delete;
        PresentBridge& operator=(PresentBridge&& copySrc) = delete;

        VkImageMemoryBarrier2 imageBarrier{};

		VkDependencyInfo dependencyInfo{};

        void SetSubresource(VkImageSubresourceRange const& subresource);
        void UpdateSrcData(Queue* lhsQueue, Resource<Image>* lhsResource, uint8_t frameIndex);
        void Execute(CommandBuffer& cmdBuf);
    };
}