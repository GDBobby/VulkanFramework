#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/CommandBuffer.h"

#include <vector>

namespace EWE{

    constexpr vkAccessFlags2 usageToAccess(vkImageUsageFlags usage, bool writes);


    struct PipelineBarrier {
        CommandBuffer& cmdBuf;

        vkPipelineStageFlagBits srcStageMask;
        vkPipelineStageFlagBits dstStageMask;
        vkDependencyFlags dependencyFlags;
        std::vector<vkMemoryBarrier> memoryBarriers;
        std::vector<vkImageMemoryBarrier> imageBarriers;
        std::vector<vkBufferMemoryBarrier> bufferBarriers;

		PipelineBarrier();
		PipelineBarrier(PipelineBarrier& copySource) noexcept;
		PipelineBarrier& operator=(PipelineBarrier& copySource) noexcept;
		PipelineBarrier(PipelineBarrier&& moveSource) noexcept;
		PipelineBarrier& operator=(PipelineBarrier&& moveSource) noexcept;

		bool Empty() const {
			return (memoryBarriers.size() + imageBarriers.size() + bufferBarriers.size()) == 0;
		}

		void AddBarrier(vkMemoryBarrier const& memoryBarrier) {
			memoryBarriers.push_back(memoryBarrier);
		}
		void AddBarrier(vkImageMemoryBarrier const& imageBarrier) {
			imageBarriers.push_back(imageBarrier);
		}
		void AddBarrier(vkBufferMemoryBarrier const& bufferBarrier) {
			bufferBarriers.push_back(bufferBarrier);
		}
		void Submit() const;

		//the parameter object passed in is no longer usable, submitting both barriers will potentially lead to errors
		void Merge(PipelineBarrier const& other);

		static void SimplifyVector(std::vector<PipelineBarrier>& barriers);
	};
	namespace Barrier {
		//this only changes the src/dst access mask
		vkImageMemoryBarrier ChangeImageLayout(
			const vkImage image, 
			const vkImageLayout oldImageLayout, 
			const vkImageLayout newImageLayout, 
			vkImageSubresourceRange const& subresourceRange
		);
		//VkImageMemoryBarrier TransitionImageLayout(VkImage& image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels, uint8_t layerCount = 1);
		
		//void TransitionImageLayoutWithBarrier(CommandBuffer cmdBuf, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImage& image, VkImageLayout srcLayout, VkImageLayout dstLayout, uint32_t mipLevels, uint8_t layerCount);

		//void TransferImageStage(CommandBuffer& cmdBuf, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, VkImage const& image);
		//void TransferImageStage(CommandBuffer& cmdBuf, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, std::vector<VkImage> const& images);
	} //namespace Barrier
} //namespace EWE
