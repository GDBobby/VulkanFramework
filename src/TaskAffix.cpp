#include "EightWinds/RenderGraph/TaskAffix.h"

#include "EightWinds/CommandBuffer.h"

namespace EWE{
	
	BarrierObject::BarrierObject(std::vector<VkImageMemoryBarrier2>&& imageBarriers, std::vector<VkBufferMemoryBarrier2>&& bufferBarriers)
	: imageBarriers{ std::forward<std::vector<VkImageMemoryBarrier2>>(imageBarriers) },
		bufferBarriers{ std::forward<std::vector<VkBufferMemoryBarrier2>>(bufferBarriers) },
		dependencyInfo{
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			.pNext = nullptr,
			.dependencyFlags = 0,
			.memoryBarrierCount = 0,
			//if this-> isn't used, it might use the forwarded/moved vector, which has had size set to 0
			.bufferMemoryBarrierCount = this->bufferBarriers.size(),
			.pBufferMemoryBarriers = this->bufferBarriers.data(),
			.imageMemoryBarrierCount = this->imageBarriers.size(),
			.pImageMemoryBarriers = this->imageBarriers.data()
		}
	{}
	
	
        //pass in the same queue if it's not a queue transfer
        //this will also compare the layout
	VkImageMemoryBarrier2 Acquire(ResourceAcquisition<Image>& img_acq, uint8_t frameIndex, Queue& rhQueue){
		
		auto& img_res = img_acq.rhs[frameIndex];
		return VkImageMemoryBarrier2{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.pNext = nullptr,
			.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
			.srcAccessMask = VK_ACCESS_2_NONE,
			.dstStageMask = img_res.usage.stage,
			.dstAccessMask = img_res.usage.accessMask,
			.oldLayout = img_res.resource->layout, 
			.newLayout = img_res.usage.layout,
			.srcQueueFamilyIndex = img_res.resource->queue->family.index,
			.dstQueueFamilyIndex = rhQueue.family.index,
			.image = img_res.resource->image,
			.subresourceRange = VkImageSubresourceRange{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = img_res.resource->mipLevels,
				.baseArrayLayer = 0,
				.layerCount = img_res.resource->arrayLayers
			}
		};
	}
	
	VkImageMemoryBarrier2 Transition(ResourceTransition<Image>& img_trans, uint8_t frameIndex, Queue& rhQueue){
		
		auto& lhs = img_trans.lhs[frameIndex];
		auto& rhs = img_trans.rhs[frameIndex];
		return VkImageMemoryBarrier2{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.pNext = nullptr,
			.srcStageMask = lhs.usage.stage,
			.srcAccessMask = lhs.usage.accessMask,
			.dstStageMask = rhs.usage.stage,
			.dstAccessMask = rhs.usage.accessMask,
			.oldLayout = lhs.usage.layout, 
			.newLayout = rhs.usage.layout,
			.srcQueueFamilyIndex = lhQueue.family.index, //is this always going to be accurate?
			.dstQueueFamilyIndex = rhQueue.family.index,
			.image = rhs.resource->image,
			.subresourceRange = VkImageSubresourceRange{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = rhs.resource->mipLevels,
				.baseArrayLayer = 0,
				.layerCount = rhs.resource->arrayLayers
			}
		};
	}

	VkBufferMemoryBarrier2 Acquire(ResourceAcquisition<Buffer>& buf_acq, uint8_t frameIndex, Queue& rhQueue) {

		auto& img_res = buf_acq.rhs[frameIndex];
		return VkBufferMemoryBarrier2{

		};
	}
	
	std::vector<VkImageMemoryBarrier2> TaskPrefix::GetImageBarriers(uint8_t frameIndex) {
		std::vector<VkImageMemoryBarrier2> imageBarriers{};
		imageBarriers.reserve(imageTransitions.size() + imageAcquisitions.size());
		
		for(auto& trans : imageTransitions){
			imageBarriers.emplace_back(Transition(trans, frameIndex, queue));
		}
		for(auto& acq : imageAcquisitions){
			imageBarriers.emplace_back(Acquire(acq, frameIndex, queue));
		}
		
		return imageBarriers;
	}
	
	std::vector<VkBufferMemoryBarrier2> TaskPrefix::GetBufferBarriers(uint8_t frameIndex){
		std::vector<VkBufferMemoryBarrier2> bufferBarriers{};
		bufferBarriers.reserve(bufferTransitions.size() + bufferAcquisitions.size());
		
		for(auto& trans : bufferTransitions){
			bufferBarriers.emplace_back(Transition(trans, frameIndex, queue));
		}
		for(auto& acq : bufferAcquisitions){
			bufferBarriers.emplace_back(Acquire(acq, frameIndex, queue));
		}
		return bufferBarriers;
	}

	
	void TaskPrefix::Execute(CommandBuffer& cmdBuf, uint8_t frameIndex){
		vkCmdPipelineBarrier2(cmdbuf, &barriers[frameIndex].dependencyInfo);
	}
	
	
	std::vector<VkImageMemoryBarrier2> TaskSuffix::GetImageBarriers(uint8_t frameIndex) {
		std::vector<VkImageMemoryBarrier2> imageBarriers{};
		imageBarriers.reserve(imageTransitions.size());
		
		for(auto& trans : imageTransitions){
			imageBarriers.emplace_back(Transition(trans, frameIndex, queue));
		}
		
		return imageBarriers;
	}
	
	std::vector<VkBufferMemoryBarrier2> TaskSuffix::GetBufferBarriers(uin8_t frameIndex){
		std::vector<VkBufferMemoryBarrier2> bufferBarriers{};
		bufferBarriers.reserve(bufferTransitions.size());
		
		for(auto& trans : bufferTransitions){
			bufferBarriers.emplace_back(Transition(trans, frameIndex, queue));
		}
		return bufferBarriers;
	}
	
	
	void TaskSuffix::Execute(CommandBuffer& cmdBuf, uint8_t frameIndex){
		vkCmdPipelineBarrier2(cmdbuf, &barriers[frameIndex].dependencyInfo);
	}
}