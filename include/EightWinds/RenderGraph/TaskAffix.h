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

    };


    struct Affix {

    };

    struct TaskPrefix{
        LogicalDevice& logicalDevice;
        Queue& queue;
        [[nodiscard]] explicit TaskPrefix(LogicalDevice& logicalDevice, Queue& queue) 
            : logicalDevice{ logicalDevice }, queue{ queue } 
        {}

        std::vector<ResourceTransition<Image>> imageTransitions;
        std::vector<ResourceAcquisition<Image>> imageAcquisitions;

        std::vector<ResourceTransition<Buffer>> bufferTransitions;
        std::vector<ResourceAcquisition<Buffer>> bufferAcquisitions;

        std::vector<VkImageMemoryBarrier2> GetImageBarriers(uint8_t frameIndex);
        std::vector<VkBufferMemoryBarrier2> GetBufferBarriers(uint8_t frameIndex);
        BarrierObject CreateBarrierObject(uint8_t frameIndex) {
            return BarrierObject{
                GetImageBarriers(frameIndex),
                GetBufferBarriers(frameIndex)
            };
        }

        PerFlight<BarrierObject> barriers;

        inline bool Empty() const noexcept {
            return (imageTransitions.size() + imageAcquisitions.size() + bufferTransitions.size() + bufferAcquisitions.size()) == 0;
        }

        void Execute(CommandBuffer& cmdBuf, uint8_t frameIndex);
    };

    //these suffixes are ONLY for queue transfers
    //if it's not a queue transfer, it ONLY needs to be put in the prefix
    //the suffix transition NEEDS to PERFECTLY MATCH the partner prefix transition
    struct TaskSuffix{
        LogicalDevice& logicalDevice;
        Queue& queue;
        [[nodiscard]] explicit TaskSuffix(LogicalDevice& logicalDevice, Queue& queue)
            : logicalDevice{ logicalDevice }, queue{ queue }
        {}

        std::vector<ResourceTransition<Image>> imageTransitions;
        std::vector<ResourceTransition<Buffer>> bufferTransitions;


        std::vector<VkImageMemoryBarrier2> GetImageBarriers(uint8_t frameIndex);
        std::vector<VkBufferMemoryBarrier2> GetBufferBarriers(uint8_t frameIndex);
        BarrierObject CreateBarrierObject(uint8_t frameIndex) {
            return BarrierObject{
                GetImageBarriers(frameIndex),
                GetBufferBarriers(frameIndex)
            };
        }
        PerFlight<BarrierObject> barriers;

        inline bool Empty() const noexcept {
            return (imageTransitions.size() + bufferTransitions.size()) == 0;
        }

        void Execute(CommandBuffer& cmdBuf, uint8_t frameIndex);
    };
}