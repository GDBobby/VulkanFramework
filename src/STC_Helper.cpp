#include "EightWinds/Backend/STC_Helper.h"

#include "EightWinds/CommandBuffer.h"
#include "EightWinds/CommandPool.h"
#include "EightWinds/Image.h"
#include "EightWinds/Buffer.h"

#include "EightWinds/Backend/Fence.h"

#include "EightWinds/Backend/PipelineBarrier.h"


namespace EWE{
	
	struct STC_Helper{
		LogicalDevice& logicalDevice;
		Queue& queue;
		EWE::CommandPool cmdPool;
		EWE::CommandBuffer cmdBuf;
		
		STC_Helper(LogicalDevice& logicalDevice, Queue& queue)
		: logicalDevice{logicalDevice},
			queue{queue},
			cmdPool{logicalDevice, queue, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT},
			cmdBuf{cmdPool.AllocateCommand(VK_COMMAND_BUFFER_LEVEL_PRIMARY)}
		{
			VkCommandBufferBeginInfo beginSTCInfo{
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
				.pNext = nullptr,
				.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
				.pInheritanceInfo = nullptr
			};
			cmdBuf.Begin(beginSTCInfo);
		}
		
		void Submit(bool wait){
			cmdBuf.End();
			
			VkCommandBufferSubmitInfo cmdBufSubmitInfo{
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
				.pNext = nullptr,
				.commandBuffer = cmdBuf
			};
			
			VkSubmitInfo2 submitInfo{
				.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
				.pNext = nullptr,
				.flags= 0,
				.waitSemaphoreInfoCount = 0,
				.pWaitSemaphoreInfos = nullptr,
				.commandBufferInfoCount = 1,
				.pCommandBufferInfos = &cmdBufSubmitInfo,
				.signalSemaphoreInfoCount = 0,
				.pSignalSemaphoreInfos = nullptr
			};
			
			if(wait){
				VkFenceCreateInfo fenceCreateInfo{
					.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
					.pNext = nullptr,
					.flags = 0
				};
				EWE::Fence stc_fence{logicalDevice, fenceCreateInfo};
				queue.Submit2(1, &submitInfo, stc_fence);
				cmdBuf.state = EWE::CommandBuffer::State::Pending;

				EWE::EWE_VK(vkWaitForFences, logicalDevice.device, 1, &stc_fence.vkFence, VK_TRUE, 5 * static_cast<uint64_t>(1.0e9));
			}
			else{
				queue.Submit2(1, &submitInfo, VK_NULL_HANDLE);
				cmdBuf.state = EWE::CommandBuffer::State::Pending;
			}
		}
	};
	
	namespace Command_Helper {
		void Transition(LogicalDevice& logicalDevice, Queue& queue, VkDependencyInfo const& dependencyInfo, bool wait) {
			STC_Helper stc_Helper(logicalDevice, queue);

			vkCmdPipelineBarrier2(stc_Helper.cmdBuf, &dependencyInfo);

			stc_Helper.Submit(wait);

		}
		void CopyBufferToImage(CommandBuffer& cmdBuf, VkBuffer buffer, Image& img, VkBufferImageCopy const& region, VkImageLayout layout){
            vkCmdCopyBufferToImage(
                cmdBuf,
                buffer,
                img.image, layout,
                1, &region
            );
		}
		void CopyBufferToImage(CommandBuffer& cmdBuf, VkBuffer vkbuffer, Image& img) {
			
            VkBufferImageCopy region{
				.bufferOffset = 0,
				.bufferRowLength = 0,
				.bufferImageHeight = 0,

				.imageSubresource = VkImageSubresourceLayers{
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.mipLevel = 0,
					.baseArrayLayer = 0,
					.layerCount = img.arrayLayers
				},

				.imageOffset = VkOffset3D{ 0, 0, 0 },
				.imageExtent = img.extent
			};
			
			CopyBufferToImage(cmdBuf, vkbuffer, img, region, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		}
		
	}

	//TransferCommandPackage::TransferCommandPackage(LogicalDevice& logicalDevice, Queue& queue)
	//	: logicalDevice{logicalDevice},
	//	queue{queue},
	//	pool{ logicalDevice, queue, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT },
	//	cmdBuf{pool.AllocateCommand(VK_COMMAND_BUFFER_LEVEL_PRIMARY) }
	//{

	//}
	
	void SingleQueueTransferContext_Image::Commands(CommandBuffer& cmdBuf){
		UsageData<Image> usage{
			.stage = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
			.accessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
			.layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
		};
		if(resource.usage.layout == VK_IMAGE_LAYOUT_GENERAL){
			usage.layout = VK_IMAGE_LAYOUT_GENERAL;
		}
		Resource<Image> lh_resource{*resource.resource[0], usage};
		
		{ //initial transition from UNDEFINED to either TRANSFER_DST_OPTIMAL or GENERAL
			auto initial_transition_barrier = Barrier::Acquire_Image(dstQueue, lh_resource, 0);
			VkDependencyInfo dependency_info{
				.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
				.pNext = nullptr,
				.memoryBarrierCount = 0,
				.bufferMemoryBarrierCount = 0,
				.imageMemoryBarrierCount = 1,
				.pImageMemoryBarriers = &initial_transition_barrier
			};
			vkCmdPipelineBarrier2(cmdBuf, &dependency_info);
		}
		Command_Helper::CopyBufferToImage(cmdBuf, stagingBuffer->buffer, *resource.resource[0], image_region, usage.layout);
		if(lh_resource.usage.layout != resource.usage.layout){
			auto ownershipBarrier = Barrier::Acquire_Image(dstQueue, resource, 0);
			
			VkDependencyInfo dependency_info{
				.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
				.pNext = nullptr,
				.memoryBarrierCount = 0,
				.bufferMemoryBarrierCount = 0,
				.imageMemoryBarrierCount = 1,
				.pImageMemoryBarriers = &ownershipBarrier
			};
			vkCmdPipelineBarrier2(cmdBuf, &dependency_info);
		}
	}
}