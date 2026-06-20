#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/RenderGraph/Resources.h"



namespace EWE{
    struct LogicalDevice;
    struct Queue;
    struct CommandBuffer;

    struct PresentBridge{
        LogicalDevice& logicalDevice;
        Queue& presentQueue;
        const std::string name;

        [[nodiscard]] explicit PresentBridge(LogicalDevice& logicalDevice, Queue& presentQueue) noexcept;

        Resource<Image>* final_swap_img_usage;;

        VkImageMemoryBarrier2 imageBarrier{};

		VkDependencyInfo dependencyInfo{};

        void UpdateSrcData(uint8_t frameIndex);
        void Execute(CommandBuffer& cmdBuf);

        PresentBridge(PresentBridge const& copySrc) = delete;
        PresentBridge(PresentBridge&& moveSrc) noexcept = delete;
        PresentBridge& operator=(PresentBridge const& copySrc) = delete;
        PresentBridge& operator=(PresentBridge&& copySrc) = delete;
    };
}