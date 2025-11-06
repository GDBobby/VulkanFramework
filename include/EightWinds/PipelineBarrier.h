#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/CommandBuffer.h"

#include <vector>

namespace EWE{

    constexpr vk::AccessFlags2 usageToAccess(vk::ImageUsageFlags usage, bool writes);


    struct PipelineBarrier {
        CommandBuffer& cmdBuf;

        vk::PipelineStageFlagBits srcStageMask;
        vk::PipelineStageFlagBits dstStageMask;
        vk::DependencyFlags dependencyFlags;
        std::vector<vk::MemoryBarrier> memoryBarriers;
        std::vector<vk::ImageMemoryBarrier> imageBarriers;
        std::vector<vk::BufferMemoryBarrier> bufferBarriers;

		PipelineBarrier();
		PipelineBarrier(PipelineBarrier& copySource) noexcept;
		PipelineBarrier& operator=(PipelineBarrier& copySource) noexcept;
		PipelineBarrier(PipelineBarrier&& moveSource) noexcept;
		PipelineBarrier& operator=(PipelineBarrier&& moveSource) noexcept;

		bool Empty() const {
			return (memoryBarriers.size() + imageBarriers.size() + bufferBarriers.size()) == 0;
		}

		void AddBarrier(vk::MemoryBarrier const& memoryBarrier) {
			memoryBarriers.push_back(memoryBarrier);
		}
		void AddBarrier(vk::ImageMemoryBarrier const& imageBarrier) {
			imageBarriers.push_back(imageBarrier);
		}
		void AddBarrier(vk::BufferMemoryBarrier const& bufferBarrier) {
			bufferBarriers.push_back(bufferBarrier);
		}
		void Submit() const;

		//the parameter object passed in is no longer usable, submitting both barriers will potentially lead to errors
		void Merge(PipelineBarrier const& other);

		static void SimplifyVector(std::vector<PipelineBarrier>& barriers);
	};
	namespace Barrier {
		//this only changes the src/dst access mask
		vk::ImageMemoryBarrier ChangeImageLayout(
			const vk::Image image, 
			const vk::ImageLayout oldImageLayout, 
			const vk::ImageLayout newImageLayout, 
			vk::ImageSubresourceRange const& subresourceRange
		);
		//VkImageMemoryBarrier TransitionImageLayout(VkImage& image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels, uint8_t layerCount = 1);
		
		//void TransitionImageLayoutWithBarrier(CommandBuffer cmdBuf, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImage& image, VkImageLayout srcLayout, VkImageLayout dstLayout, uint32_t mipLevels, uint8_t layerCount);

		//void TransferImageStage(CommandBuffer& cmdBuf, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, VkImage const& image);
		//void TransferImageStage(CommandBuffer& cmdBuf, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, std::vector<VkImage> const& images);
	} //namespace Barrier
} //namespace EWE
}