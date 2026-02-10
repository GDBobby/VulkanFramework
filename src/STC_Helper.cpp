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
            EWE_VK(vkCmdCopyBufferToImage,
                cmdBuf,
                buffer,
                image, layout,
                1, &region
            );
		}
		void CopyBufferToImage(CommandBuffer& cmdBuf, VkBuffer buffer, Image& img){
			
            VkBufferImageCopy region{
				.bufferOffset = 0;
				.bufferRowLength = 0;
				.bufferImageHeight = 0;

				.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				.imageSubresource.mipLevel = 0;
				.imageSubresource.baseArrayLayer = 0;
				.imageSubresource.layerCount = img.layerCount;

				.imageOffset = { 0, 0, 0 };
				.imageExtent = VkExtent3D{
					.width = img.width,
					.height = img.height,
					1
				};
			};
			
			CopyBufferToImage(cmdBuf, img, region, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		}
		
	}
	
	TransferCommandPackage* TransferContext<Image>::Commands(){
		
		Queue& firstQueue = transferQueue ? *transferQueue : dstQueue;
		
		TransferCommandPackage ret = new TransferCommandPackage{
			.commandPool{img.logicalDevice, firstQueue, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT},
			.cmdBuf{commandPool.AllocateCommand(VK_COMMAND_BUFFER_LEVEL_PRIMARY)},
		};
		cmdBuf.Begin();
		
		//if the destination is either compute, or generating mipmaps, put the layout to GENERAL
		UsageData<Image> usage{
			.stage = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
			.accessMask = VK_ACCESS_2_IDK,
			.layout = dstLayout
		};
		Resource<Image> resource{img, usage};
		
		ret->barrier = Barrier::Acquire_Image(firstQueue, resource, 0);
		
		return ret;
	}
}