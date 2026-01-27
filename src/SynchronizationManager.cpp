#include "EightWinds/RenderGraph/SynchronizationManager.h"

#include "EightWinds/RenderGraph/GPUTask.h"
#include "EightWinds/Image.h"
#include "EightWinds/Buffer.h"


namespace EWE{
    void SynchronizationManager::AddTransition_Buffer(GPUTask& lhs, uint32_t lh_index, GPUTask& rhs, uint32_t rh_index) {
        buffer_transitions.emplace_back(
            TransitionObjects<Buffer>{
                .lhs = &lhs,
                .lh_index = lh_index,
                .rhs = &rhs,
                .rh_index = rh_index
            }
        );
    }

    void SynchronizationManager::AddTransition_Image(GPUTask& lhs, uint32_t lh_index, GPUTask& rhs, uint32_t rh_index) {
        image_transitions.emplace_back(
            TransitionObjects<Image>{
                .lhs = &lhs,
                .lh_index = lh_index,
                .rhs = &rhs,
                .rh_index = rh_index
            }
        );
    }
    void SynchronizationManager::AddAcquisition_Buffer(GPUTask& rhs, uint32_t rh_index) {
        buffer_acquisitions.emplace_back(
            AcquireObjects<Buffer>{
                .rhs = &rhs,
                .rh_index = rh_index
            }
        );
    }
    void SynchronizationManager::AddAcquisition_Image(GPUTask& rhs, uint32_t rh_index) {
        image_acquisitions.emplace_back(
            AcquireObjects<Image>{
                .rhs = &rhs,
                .rh_index = rh_index
            }
        );
    }
	
	
	
	
	
	
	
        //pass in the same queue if it's not a queue transfer
        //this will also compare the layout
	VkImageMemoryBarrier2 Acquire_Image(GPUTask& rhs_task, uint32_t rh_index, uint8_t frameIndex){
		
		auto& img_res = rhs_task.resources.images[rh_index];
		const uint32_t srcQueueFamilyIndex = img_res.resource[frameIndex]->owningQueue ? img_res.resource[frameIndex]->owningQueue->family.index : rhs_task.queue.family.index;
		return VkImageMemoryBarrier2{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.pNext = nullptr,
			.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
			.srcAccessMask = VK_ACCESS_2_NONE,
			.dstStageMask = img_res.usage.stage,
			.dstAccessMask = img_res.usage.accessMask,
			.oldLayout = img_res.resource[frameIndex]->layout,
			.newLayout = img_res.usage.layout,
			.srcQueueFamilyIndex = srcQueueFamilyIndex,
			.dstQueueFamilyIndex = rhs_task.queue.family.index,
			.image = img_res.resource[frameIndex]->image,
			.subresourceRange = VkImageSubresourceRange{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = img_res.resource[frameIndex]->mipLevels,
				.baseArrayLayer = 0,
				.layerCount = img_res.resource[frameIndex]->arrayLayers
			}
		};
	}
	
	VkImageMemoryBarrier2 Transition_Image(GPUTask& lhs_task, uint32_t lh_index, GPUTask& rhs_task, uint32_t rh_index, uint8_t frameIndex){
		
		auto& lhs_res = lhs_task.resources.images[lh_index];
		auto& rhs_res = rhs_task.resources.images[rh_index];
		return VkImageMemoryBarrier2{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.pNext = nullptr,
			.srcStageMask = lhs_res.usage.stage,
			.srcAccessMask = lhs_res.usage.accessMask,
			.dstStageMask = rhs_res.usage.stage,
			.dstAccessMask = rhs_res.usage.accessMask,
			.oldLayout = lhs_res.usage.layout, 
			.newLayout = rhs_res.usage.layout,
			.srcQueueFamilyIndex = lhs_task.queue.family.index,
			.dstQueueFamilyIndex = rhs_task.queue.family.index,
			.image = rhs_res.resource[frameIndex]->image,
			.subresourceRange = VkImageSubresourceRange{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = rhs_res.resource[frameIndex]->mipLevels,
				.baseArrayLayer = 0,
				.layerCount = rhs_res.resource[frameIndex]->arrayLayers
			}
		};
	}
	
	
	VkBufferMemoryBarrier2 Acquire_Buffer(GPUTask& rhs_task, uint32_t rh_index, uint8_t frameIndex){
		
		auto& buf_res = rhs_task.resources.buffers[rh_index];
		const uint32_t srcQueueFamilyIndex = buf_res.resource[frameIndex]->owningQueue ? buf_res.resource[frameIndex]->owningQueue->family.index : rhs_task.queue.family.index;
		return VkBufferMemoryBarrier2{
			.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
			.pNext = nullptr,
			.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
			.srcAccessMask = VK_ACCESS_2_NONE,
			.dstStageMask = buf_res.usage.stage,
			.dstAccessMask = buf_res.usage.accessMask,
			.srcQueueFamilyIndex = srcQueueFamilyIndex, //if this is nullptr, this field needs to be equal to dstQueueFamilyIndex
			.dstQueueFamilyIndex = rhs_task.queue.family.index,
			.buffer = buf_res.resource[frameIndex]->buffer_info.buffer,
			.offset = 0,
			.size = buf_res.resource[frameIndex]->bufferSize
		};
	}
	
	VkBufferMemoryBarrier2 Transition_Buffer(GPUTask& lhs_task, uint32_t lh_index, GPUTask& rhs_task, uint32_t rh_index, uint8_t frameIndex){
		
		auto& lhs_buf = lhs_task.resources.buffers[lh_index];
		auto& rhs_buf = rhs_task.resources.buffers[rh_index];
		return VkBufferMemoryBarrier2{
			.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
			.pNext = nullptr,
			.srcStageMask = lhs_buf.usage.stage,
			.srcAccessMask = lhs_buf.usage.accessMask,
			.dstStageMask = rhs_buf.usage.stage,
			.dstAccessMask = rhs_buf.usage.accessMask,
			.srcQueueFamilyIndex = lhs_task.queue.family.index,
			.dstQueueFamilyIndex = rhs_task.queue.family.index,
			.buffer = rhs_buf.resource[frameIndex]->buffer_info.buffer,
			.offset = 0,
			.size = rhs_buf.resource[frameIndex]->bufferSize
		};
	}
	
	

    void SynchronizationManager::PopulateAffixes(uint8_t frameIndex){

		//all barriers need to have been cleared before populating, 
		// potentially put validaiton in here that all touched tasks have 0 existing barriers
		for (auto& trans : buffer_transitions) {
			auto const& barr = Transition_Buffer(*trans.lhs, trans.lh_index, *trans.rhs, trans.rh_index, frameIndex);
			if (trans.lhs->queue != trans.rhs->queue) {
				//put a suffix on lhs
				trans.lhs->suffix.barriers[frameIndex].bufferBarriers.push_back(barr);
			}
			trans.rhs->prefix.barriers[frameIndex].bufferBarriers.push_back(barr);
		}
		for (auto& trans : image_transitions) {
			auto const& barr = Transition_Image(*trans.lhs, trans.lh_index, *trans.rhs, trans.rh_index, frameIndex);
			if (trans.lhs->queue != trans.rhs->queue) {
				//put a suffix on lhs
				trans.lhs->suffix.barriers[frameIndex].imageBarriers.push_back(barr);
			}
			trans.rhs->prefix.barriers[frameIndex].imageBarriers.push_back(barr);

		}
		for (auto& acq : buffer_acquisitions) {
			acq.rhs->prefix.barriers[frameIndex].bufferBarriers.push_back(Acquire_Buffer(*acq.rhs, acq.rh_index, frameIndex));
		}
		for (auto& acq : image_acquisitions) {
			acq.rhs->prefix.barriers[frameIndex].imageBarriers.push_back(Acquire_Image(*acq.rhs, acq.rh_index, frameIndex));

		}
		
    }
}