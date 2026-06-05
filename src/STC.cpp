#include "EightWinds/Command/STC.h"

#include "EightWinds/Queue.h"

#include "EightWinds/Backend/PipelineBarrier.h"
#include "EightWinds/Command/STC_Helper.h"

namespace EWE{
void TransferContext<Image>::Acquire(CommandBuffer& cmdBuf, Queue& acqQueue, UsageData<Image> const& initial_usage){
		RuntimeArray<VkImageMemoryBarrier2> initial_transition_barriers{images.Size()};
		for(std::size_t i = 0; i < images.Size(); i++){
			initial_transition_barriers[i] = Barrier::Acquire_Image(acqQueue, *images[i], initial_usage);
		}
		VkDependencyInfo dependency_info{
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			.pNext = nullptr,
			.memoryBarrierCount = 0,
			.bufferMemoryBarrierCount = 0,
			.imageMemoryBarrierCount = static_cast<std::uint32_t>(initial_transition_barriers.Size()),
			.pImageMemoryBarriers = initial_transition_barriers.Data()
		};
		vkCmdPipelineBarrier2(cmdBuf, &dependency_info);
	}
	void TransferContext<Image>::Stage(CommandBuffer& cmdBuf, UsageData<Image> const& initial_usage){
		for(std::size_t i = 0; i < images.Size(); i++){
			Command_Helper::CopyBufferToImage(cmdBuf, stagingBuffer, *images[i], regions[i], initial_usage.layout);
		}
	}
	RuntimeArray<VkImageMemoryBarrier2> TransferContext<Image>::ChangeOwnership(CommandBuffer& cmdBuf, Queue& acqQueue, Queue& dstQueue, UsageData<Image> const& initial_usage){
		RuntimeArray<VkImageMemoryBarrier2> ownerBarriers{images.Size()};
		for(std::size_t i = 0; i < images.Size(); i++){
			ownerBarriers[i] = Barrier::Transition_Image(acqQueue, *images[i], dstQueue, initial_usage, final_usage);
		}
		VkDependencyInfo dependency_info{
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			.pNext = nullptr,
			.memoryBarrierCount = 0,
			.bufferMemoryBarrierCount = 0,
			.imageMemoryBarrierCount = static_cast<uint32_t>(ownerBarriers.Size()),
			.pImageMemoryBarriers = ownerBarriers.Data()
		};
		vkCmdPipelineBarrier2(cmdBuf, &dependency_info);

		return ownerBarriers;
	}

	void TransferContext<Buffer>::Acquire(CommandBuffer& cmdBuf, Queue& acqQueue, UsageData<Buffer> const& initial_usage){
		RuntimeArray<VkBufferMemoryBarrier2> initial_transition_barriers{buffers.Size()};
		for(std::size_t i = 0; i < buffers.Size(); i++){
			initial_transition_barriers[i] = Barrier::Acquire_Buffer(acqQueue, *buffers[i], initial_usage);
		}
		VkDependencyInfo dependency_info{
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			.pNext = nullptr,
			.memoryBarrierCount = 0,
			.bufferMemoryBarrierCount = static_cast<uint32_t>(initial_transition_barriers.Size()),
			.pBufferMemoryBarriers = initial_transition_barriers.Data(),
			.imageMemoryBarrierCount = 0
		};
		vkCmdPipelineBarrier2(cmdBuf, &dependency_info);
	}
	void TransferContext<Buffer>::Stage(CommandBuffer& cmdBuf){
		if(buffers.Size() == regions.Size()){
			for(std::size_t i = 0; i < buffers.Size(); i++) {
				Command_Helper::CopyBufferToBuffer(cmdBuf, stagingBuffer, *buffers[i], regions[i]);
			}
		}
		else{
			EWE_ASSERT(regions.Size() == 1);
			if(buffers.Size() != max_frames_in_flight){
				Log::Warning("copying data to multiple buffers not at a count of max_frames_in_flight\n");
			}
			for(auto& buf : buffers) {
				Command_Helper::CopyBufferToBuffer(cmdBuf, stagingBuffer, *buf, regions[0]);
			}
		}
	}
	RuntimeArray<VkBufferMemoryBarrier2> TransferContext<Buffer>::ChangeOwnership(CommandBuffer& cmdBuf, Queue& acqQueue, Queue& rh_queue, UsageData<Buffer> const& initial_usage){
		RuntimeArray<VkBufferMemoryBarrier2> ownerBarriers{buffers.Size()};
		for(std::size_t i = 0; i < buffers.Size(); i++){
			ownerBarriers[i] = VkBufferMemoryBarrier2{
				.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
				.pNext = nullptr,
				.srcStageMask = initial_usage.stage,
				.srcAccessMask = initial_usage.accessMask,
				.dstStageMask = final_usage.stage,
				.dstAccessMask = final_usage.accessMask,
				.srcQueueFamilyIndex = acqQueue.FamilyIndex(),
				.dstQueueFamilyIndex = rh_queue.FamilyIndex(),
				.buffer = buffers[i]->buffer_info.buffer,
				.offset = 0,
				.size = stagingBuffer.bufferSize
			};
		}
		VkDependencyInfo dependency_info{
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			.pNext = nullptr,
			.memoryBarrierCount = 0,
			.bufferMemoryBarrierCount = static_cast<uint32_t>(ownerBarriers.Size()),
			.pBufferMemoryBarriers = ownerBarriers.Data(),
			.imageMemoryBarrierCount = 0
		};
		vkCmdPipelineBarrier2(cmdBuf, &dependency_info);
		return ownerBarriers;
	}
} //namespace EWE