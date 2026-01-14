#include "EightWinds/RenderGraph/TaskBridge.h"

#include "EightWinds/ImageView.h"
#include "EightWinds/CommandBuffer.h"

	/*
namespace EWE {

	template<typename T>
	inline void AddUniqueResource(std::vector<T*>& v, T* ptr) {
		if (ptr == nullptr) {
			return;
		}
		for (auto p : v) {
			if (p == ptr) {
				return;
			}
		}
		v.push_back(ptr);
	}

	TaskBridge::TaskBridge(GPUTask& lhs, GPUTask& rhs) noexcept
		: logicalDevice{ lhs.logicalDevice }, queue{ rhs.queue },
		lhs{ &lhs },
		rhs{ &rhs },
		name{lhs.name + " -> " + rhs.name}
	{
		dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dependencyInfo.pNext = nullptr;
		dependencyInfo.dependencyFlags = 0; //might need to fine tune this
	}
	TaskBridge::TaskBridge(GPUTask& rhs) noexcept
		: logicalDevice{ rhs.logicalDevice }, queue{ rhs.queue },
		lhs{nullptr},
		rhs{ &rhs },
		name{std::string(":~ " + rhs.name)}
	{
		dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dependencyInfo.pNext = nullptr;
		dependencyInfo.dependencyFlags = 0; //might need to fine tune this
	}

	TaskBridge::TaskBridge(
		LogicalDevice& logicalDevice,
		std::vector<Resource<Buffer>*>& rhsBuffs,
		std::vector<Resource<Image>*>& rhsImgs,
		Queue& rhQueue
	) : logicalDevice{ logicalDevice }, queue{rhQueue},
		lhs{ nullptr }, rhs{ nullptr },
		name{ "explicit rh only bridge" }
	{
		dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dependencyInfo.pNext = nullptr;
		dependencyInfo.dependencyFlags = 0; //might need to fine tune this
		GenerateRightHandBarriers(rhsBuffs, rhsImgs, rhQueue);
	}

	TaskBridge::TaskBridge(
		LogicalDevice& logicalDevice, Queue& queue,
		std::vector<Resource<Buffer>*>& lhsBuffs, std::vector<Resource<Buffer>*>& rhsBuffs,
		std::vector<Resource<Image>*>& lhsImgs, std::vector<Resource<Image>*>& rhsImgs,
		Queue& lhQueue, Queue& rhQueue
	) : logicalDevice{ logicalDevice }, queue{ queue },
		lhs{ nullptr }, rhs{ nullptr },
		name{ "explicit lh:rh bridge" }
	{
		dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dependencyInfo.pNext = nullptr;
		dependencyInfo.dependencyFlags = 0; //might need to fine tune this
		GenerateBridgeBarriers(
			lhsBuffs, rhsBuffs,
			lhsImgs, rhsImgs,
			lhQueue, rhQueue
		);
	}

	TaskBridge::TaskBridge(TaskBridge&& moveSrc) noexcept
		: logicalDevice{ moveSrc.logicalDevice }, queue{moveSrc.queue},
		lhs{ moveSrc.lhs },
		rhs{ moveSrc.rhs },
		bufferBarriers{ std::move(moveSrc.bufferBarriers) },
		imageBarriers{ std::move(moveSrc.imageBarriers) },
		bufferBarrierResources{ std::move(moveSrc.bufferBarrierResources) },
		imageBarrierResources{ std::move(moveSrc.imageBarrierResources) }
	{
		dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dependencyInfo.pNext = nullptr;
		dependencyInfo.dependencyFlags = 0; //might need to fine tune this
	}

	void TaskBridge::RecreateBarriers() {
		bufferBarriers.clear();
		imageBarriers.clear();
		bufferBarrierResources.clear();
		imageBarrierResources.clear();
		if (lhs != nullptr) {
			GenerateBridgeBarriers(
				lhs->explicitBufferState, rhs->explicitBufferState,
				lhs->explicitImageState, rhs->explicitImageState,
				lhs->queue, rhs->queue
			);
		}
		else {
			GenerateRightHandBarriers(
				rhs->explicitBufferState,
				rhs->explicitImageState,
				rhs->queue
			);
		}
	}

	void TaskBridge::GenerateRightHandBarriers(
		std::vector<Resource<Buffer>*>& buffs,
		std::vector<Resource<Image>*>& imgs,
		Queue& rhQueue
	) {
		{ //buffers
			VkBufferMemoryBarrier2 buf_bar{};
			buf_bar.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
			buf_bar.pNext = nullptr;
			buf_bar.srcQueueFamilyIndex = rhQueue.family.index;
			buf_bar.dstQueueFamilyIndex = rhQueue.family.index;

			//layout exclusive transitions
			for (auto& rhsBuf: buffs) {
				bool needs_barrier = rhsBuf->buffer->owningQueue->family.index != rhQueue.family.index;
				if (needs_barrier) {
					//add to barriers
					bool foundMatch = false;
					for (auto& existing : bufferBarriers) {
						if (existing.buffer == rhsBuf->buffer->buffer_info.buffer) {
							foundMatch = true;
							break;
						}
					}
					if (foundMatch) {
						continue;
					}

					buf_bar.srcQueueFamilyIndex = rhsBuf->buffer->owningQueue->family.index;

					buf_bar.srcAccessMask = VK_ACCESS_2_NONE; //i dont really know what to put here
					buf_bar.dstAccessMask = rhsBuf->usage.accessMask;
					buf_bar.srcStageMask = rhsBuf->usage.stage;
					buf_bar.dstStageMask = rhsBuf->usage.stage;

					buf_bar.buffer = rhsBuf->buffer->buffer_info.buffer;
					buf_bar.size = rhsBuf->buffer->bufferSize;
					buf_bar.offset = rhsBuf->buffer->buffer_info.offset;


					bufferBarriers.emplace_back(buf_bar);
					bufferBarrierResources.push_back(
						BarrierResource<Buffer>{
						.resource = rhsBuf->buffer
					}
					);
				}
			}
		}


		{ //images
			VkImageMemoryBarrier2 img_bar{};
			img_bar.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
			img_bar.pNext = nullptr;
			img_bar.srcQueueFamilyIndex = rhQueue.family.index;
			img_bar.dstQueueFamilyIndex = rhQueue.family.index;

			img_bar.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			img_bar.subresourceRange.baseArrayLayer = 0;
			img_bar.subresourceRange.baseMipLevel = 0;

			//layout exclusive transitions
			for (auto& rhsImage : imgs) {
				bool queueTransfer = rhsImage->image->owningQueue ? (rhsImage->image->owningQueue->family.index != rhQueue.family.index) : false; 
				bool needs_barrier = (rhsImage->image->layout != rhsImage->usage.layout) || queueTransfer;
				if (needs_barrier) {
					//add to barriers
					bool foundMatch = false;
					for (auto& existing : imageBarriers) {
						if (existing.image == rhsImage->image->image) {
#if EWE_DEBUG_BOOL
							assert(existing.newLayout == rhsImage->usage.layout); //need to rectify this if it's an isuse
#endif
							foundMatch = true;
							break;
						}
					}
					if (foundMatch) {
						continue;
					}

					if (rhsImage->image->owningQueue != nullptr) {
						img_bar.srcQueueFamilyIndex = rhsImage->image->owningQueue->family.index;
					}
					else {
						img_bar.srcQueueFamilyIndex = rhQueue.family.index; //same as QUEUE_FAMILY_UNKNOWN if theyre both equal
					}

					img_bar.oldLayout = rhsImage->image->layout;
					img_bar.newLayout = rhsImage->usage.layout;
					img_bar.srcAccessMask = VK_ACCESS_2_NONE; //i dont really know what to put here
					img_bar.dstAccessMask = rhsImage->usage.accessMask;
					img_bar.srcStageMask = rhsImage->usage.stage;
					img_bar.dstStageMask = rhsImage->usage.stage;
					img_bar.image = rhsImage->image->image;
					img_bar.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					img_bar.subresourceRange.layerCount = rhsImage->image->arrayLayers;
					img_bar.subresourceRange.levelCount = rhsImage->image->mipLevels;

					imageBarriers.emplace_back(img_bar);
					imageBarrierResources.push_back(
						BarrierResource<Image>{
						.resource = rhsImage->image,
						.finalLayout = rhsImage->usage.layout
					}
					);
				}
			}
		}
	}

	void TaskBridge::GenerateBridgeBarriers(
		std::vector<Resource<Buffer>*>& lhsBuffs, std::vector<Resource<Buffer>*>& rhsBuffs,
		std::vector<Resource<Image>*>& lhsImgs, std::vector<Resource<Image>*>& rhsImgs,
		Queue& lhQueue, Queue& rhQueue
	) {

		std::vector<Resource<Buffer>*> rhsOnlyBuffer;
		std::vector<Resource<Image>*> rhsOnlyImg;
		{ //buffers
			VkBufferMemoryBarrier2 buf_bar{};
			buf_bar.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
			buf_bar.pNext = nullptr;
			buf_bar.srcQueueFamilyIndex = rhQueue.family.index;
			buf_bar.dstQueueFamilyIndex = rhQueue.family.index;

			//layout exclusive transitions
			for (auto& rhsBuf : rhsBuffs) {
				bool foundLHMatch = false;
				for (auto& lhsBuf : lhsBuffs) {
					if (lhsBuf->buffer == rhsBuf->buffer) {
						foundLHMatch = true;
						if (
							(rhsBuf->buffer->owningQueue->family.index != rhQueue.family.index) ||
							(lhQueue.family.index != rhQueue.family.index) ||
							(GetAccessMaskWrite(lhsBuf->usage.accessMask) || GetAccessMaskWrite(rhsBuf->usage.accessMask))

						) {

							buf_bar.srcQueueFamilyIndex = rhsBuf->buffer->owningQueue->family.index;

							buf_bar.srcAccessMask = rhsBuf->usage.accessMask; //i dont really know what to put here
							buf_bar.dstAccessMask = rhsBuf->usage.accessMask;
							buf_bar.srcStageMask = lhsBuf->usage.stage;
							buf_bar.dstStageMask = rhsBuf->usage.stage;

							buf_bar.buffer = rhsBuf->buffer->buffer_info.buffer;
							buf_bar.size = rhsBuf->buffer->bufferSize;
							buf_bar.offset = rhsBuf->buffer->buffer_info.offset;


							bufferBarriers.emplace_back(buf_bar);
							bufferBarrierResources.push_back(
								BarrierResource<Buffer>{
								.resource = rhsBuf->buffer
								}
							);
							break;//guaranteed to be unique
						}
					}
				}
					//failed to find a match
				if (!foundLHMatch) {
					rhsOnlyBuffer.push_back(rhsBuf);
				}
			}
		}
		

		{ //images
			VkImageMemoryBarrier2 img_bar{};
			img_bar.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
			img_bar.pNext = nullptr;
			img_bar.srcQueueFamilyIndex = lhQueue.family.index;
			img_bar.dstQueueFamilyIndex = rhQueue.family.index;

			img_bar.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			img_bar.subresourceRange.baseArrayLayer = 0;
			img_bar.subresourceRange.baseMipLevel = 0;

			//layout exclusive transitions
			for (auto& rhsImage : rhsImgs) {
				bool foundLHMatch = false;
				for (auto& lhsImage : lhsImgs) {
					if (lhsImage->image == rhsImage->image) {
						foundLHMatch = true;
						if (
							(lhsImage->usage.layout != rhsImage->usage.layout) ||
							(rhsImage->image->owningQueue->family.index != rhQueue.family.index) ||
							(GetAccessMaskWrite(lhsImage->usage.accessMask) || GetAccessMaskWrite(rhsImage->usage.accessMask))
						) {
									//add to barriers
							img_bar.oldLayout = lhsImage->usage.layout;
							img_bar.newLayout = rhsImage->usage.layout;
							img_bar.srcAccessMask = lhsImage->usage.accessMask; //i dont really know what to put here
							img_bar.dstAccessMask = rhsImage->usage.accessMask;
							img_bar.srcStageMask = lhsImage->usage.stage;
							img_bar.dstStageMask = rhsImage->usage.stage;
							img_bar.image = rhsImage->image->image;
							img_bar.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
							img_bar.subresourceRange.layerCount = rhsImage->image->arrayLayers;
							img_bar.subresourceRange.levelCount = rhsImage->image->mipLevels;

							imageBarriers.emplace_back(img_bar);
							imageBarrierResources.push_back(
								BarrierResource<Image>{
								.resource = rhsImage->image,
								.finalLayout = rhsImage->usage.layout
								}
							);
						}
						break; //these are guaranteed to be unique
					}
				}
				//failed to find a match
				if (!foundLHMatch) {
					rhsOnlyImg.push_back(rhsImage);
				}
			}
		}

		GenerateRightHandBarriers(rhsOnlyBuffer, rhsOnlyImg, rhQueue);
	}


	void TaskBridge::Execute(CommandBuffer& cmdBuf){
#if EWE_DEBUG_BOOL
		if (rhs != nullptr) {
			assert(cmdBuf.commandPool.queue == rhs->queue);
		}
#endif

		dependencyInfo.memoryBarrierCount = 0; //i could move this into the constructor but idk what this is even for tbh
		dependencyInfo.imageMemoryBarrierCount = static_cast<uint32_t>(imageBarriers.size());
		dependencyInfo.bufferMemoryBarrierCount = static_cast<uint32_t>(bufferBarriers.size());

		dependencyInfo.pMemoryBarriers = nullptr;
		dependencyInfo.pImageMemoryBarriers = imageBarriers.data();
		dependencyInfo.pBufferMemoryBarriers = bufferBarriers.data();

		const uint32_t totalSize = dependencyInfo.memoryBarrierCount + dependencyInfo.imageMemoryBarrierCount + dependencyInfo.bufferMemoryBarrierCount;
		if(totalSize > 0){
#if EWE_DEBUG_BOOL
			assert(!cmdBuf.debug_currentlyRendering);
#endif
			vkCmdPipelineBarrier2(cmdBuf, &dependencyInfo);		
		}

		for (auto& bbr : bufferBarrierResources) {
			bbr.resource->owningQueue = &rhs->queue;
		}

		for (auto& ibr : imageBarrierResources) {
			ibr.resource->layout = ibr.finalLayout;
			if (rhs != nullptr) {
				ibr.resource->owningQueue = &rhs->queue;
			}
		}
	}
}
*/