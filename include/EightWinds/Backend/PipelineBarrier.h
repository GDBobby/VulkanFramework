#pragma once

#include "EWGraphics/Vulkan/VulkanHeader.h"

#include <vector>

namespace EWE {
    struct PipelineBarrier {
        std::vector<VkMemoryBarrier2> memoryBarriers;
        std::vector<VkImageMemoryBarrier2> imageBarriers;
        std::vector<VkBufferMemoryBarrier2> bufferBarriers;
		
        VkDependencyInfo dependencyInfo;

		[[nodiscard]] PipelineBarrier();
		[[nodiscard]] PipelineBarrier(PipelineBarrier& copySource);
		PipelineBarrier& operator=(PipelineBarrier& copySource);
		[[nodiscard]] PipelineBarrier(PipelineBarrier&& moveSource) noexcept;
		PipelineBarrier& operator=(PipelineBarrier&& moveSource) noexcept;
		
		//im not going to currently bother with raw memory barriers, i havent used or seen one used yet
        [[nodiscard]] explicit PipelineBarrier(std::vector<VkImageMemoryBarrier2>&& imageBarriers);
        [[nodiscard]] explicit PipelineBarrier(std::vector<VkImageMemoryBarrier2>&& imageBarriers, std::vector<VkBufferMemoryBarrier2>&& bufferBarriers);
        [[nodiscard]] explicit PipelineBarrier(std::vector<VkBufferMemoryBarrier2>&& bufferBarriers);
		
		void FixPointers();
		
		bool Empty() const {
			return (memoryBarriers.size() + imageBarriers.size() + bufferBarriers.size()) == 0;
		}

		void AddBarrier(VkMemoryBarrier2 const& memoryBarrier) {
			memoryBarriers.push_back(memoryBarrier);
		}
		void AddBarrier(VkImageMemoryBarrier2 const& imageBarrier) {
			imageBarriers.push_back(imageBarrier);
		}
		void AddBarrier(VkBufferMemoryBarrier2 const& bufferBarrier) {
			bufferBarriers.push_back(bufferBarrier);
		}

		//the parameter object passed in is no longer usable, submitting both barriers will potentially lead to errors
		void Merge(PipelineBarrier const& other);
	};
	namespace Barrier {
		VkImageMemoryBarrier2 Acquire_Image(Queue& dstQueue, Resource<Image>& resource, uint8_t frameIndex);
		VkBufferMemoryBarrier2 Acquire_Buffer(Queue& dstQueue, Resource<Buffer>& resource, uint8_t frameIndex);
		
		VkImageMemoryBarrier2 Transition_Image(Queue& srcQueue, Resource<Image>& lh_resource, Queue& dstQueue, Resource<Image>& rh_resource, uint8_t frameIndex);
		VkBufferMemoryBarrier2 Transition_Buffer(Queue& srcQueue, Resource<Buffer>& lh_resource, Queue& dstQueue, Resource<Buffer>& rh_resource, uint8_t frameIndex);
		
	} //namespace Barrier
} //namespace EWE
