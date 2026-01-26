#include "EightWinds/Backend/STC_Helper.h"

#include "EightWinds/CommandBuffer.h"
#include "EightWinds/CommandPool.h"
#include "EightWinds/Image.h"
#include "EightWinds/Buffer.h"

#include "EightWinds/Backend/Fence.h"


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
	
	
	void Transition(LogicalDevice& logicalDevice, Queue& queue, VkDependencyInfo const& dependencyInfo, bool wait){
        STC_Helper stc_Helper(logicalDevice, queue);

		vkCmdPipelineBarrier2(stc_Helper.cmdBuf, &dependencyInfo);
		
		stc_Helper.Submit(wait);
		
	}
}