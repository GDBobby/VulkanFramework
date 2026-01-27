#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/RenderGraph/Resources.h"
#include "EightWinds/LogicalDevice.h"
#include "EightWinds/Queue.h"

namespace EWE{
	
    struct BarrierObject {
        std::vector<VkImageMemoryBarrier2> imageBarriers;
        std::vector<VkBufferMemoryBarrier2> bufferBarriers;
        VkDependencyInfo dependencyInfo;

        [[nodiscard]] BarrierObject() : imageBarriers{}, bufferBarriers{}, dependencyInfo{} {}
        [[nodiscard]] explicit BarrierObject(std::vector<VkImageMemoryBarrier2>&& imageBarriers, std::vector<VkBufferMemoryBarrier2>&& bufferBarriers);

        inline bool Empty() const {
            return (imageBarriers.size() == 0) && (bufferBarriers.size() == 0);
        }
    };

    struct CommandBuffer;

    struct TaskAffix {

        PerFlight<BarrierObject> barriers{};

        inline bool Empty() const noexcept {
            for (uint8_t i = 0; i < max_frames_in_flight; i++) {
                if (!barriers[i].Empty()) {
                    return false;
                }
            }
            return true;
        }

        void Clear(uint8_t frameIndex) {
            barriers[frameIndex].bufferBarriers.clear();
            barriers[frameIndex].imageBarriers.clear();
        }
        void Clear() {
            for (uint8_t i = 0; i < max_frames_in_flight; i++) {
                Clear(i);
            }
        }

        void Execute(CommandBuffer& cmdBuf, uint8_t frameIndex);
    };
}