#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/RenderGraph/Resources.h"
#include "EightWinds/LogicalDevice.h"
#include "EightWinds/Queue.h"

namespace EWE{
	
    struct BarrierObject {
        std::vector<VkMemoryBarrier2> memoryBarriers;
        std::vector<VkImageMemoryBarrier2> imageBarriers;
        std::vector<VkBufferMemoryBarrier2> bufferBarriers;
        VkDependencyInfo dependencyInfo;

        [[nodiscard]] BarrierObject();
        [[nodiscard]] explicit BarrierObject(std::vector<VkImageMemoryBarrier2>&& imageBarriers, std::vector<VkBufferMemoryBarrier2>&& bufferBarriers);

        void FixPointers();

        inline bool Empty() const {
            return (imageBarriers.size() == 0) && (bufferBarriers.size() == 0 && (memoryBarriers.size() == 0));
        }
    };

    struct CommandBuffer;

    struct TaskAffix {

        struct ImageLayoutUpdate {
            Image* img;
            VkImageLayout layout;
        };

        std::vector<ImageLayoutUpdate> image_updates;

        PerFlight<BarrierObject> barriers{};

        bool Empty() const noexcept;

        void Clear(uint8_t frameIndex);
        void Clear();

        void Execute(CommandBuffer& cmdBuf, uint8_t frameIndex);
    };
}