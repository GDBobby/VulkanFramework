#include "EightWinds/RenderGraph/PresentBridge.h"

#include "EightWinds/CommandBuffer.h"
#include "EightWinds/Image.h"

namespace EWE{
    PresentBridge::PresentBridge(LogicalDevice& _logicalDevice, Queue& _presentQueue) noexcept
    : logicalDevice{_logicalDevice},
        presentQueue{_presentQueue},
        name{"present bridge"},
        final_swap_img_usage{nullptr}
    {
		dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dependencyInfo.pNext = nullptr;
		dependencyInfo.dependencyFlags = 0; //might need to fine tune this
        
		dependencyInfo.memoryBarrierCount = 0; //i could move this into the constructor but idk what this is even for tbh
		dependencyInfo.imageMemoryBarrierCount = 1;
		dependencyInfo.bufferMemoryBarrierCount = 0;

        imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        imageBarrier.pNext = nullptr;

        imageBarrier.dstQueueFamilyIndex = presentQueue.family.index;
        imageBarrier.dstAccessMask = VK_ACCESS_2_NONE; //idk what to put here
        imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
        imageBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        imageBarrier.subresourceRange = VkImageSubresourceRange{
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        };

		dependencyInfo.pMemoryBarriers = nullptr;
		dependencyInfo.pImageMemoryBarriers = &imageBarrier;
		dependencyInfo.pBufferMemoryBarriers = nullptr;
    }
    
    void PresentBridge::UpdateSrcData(uint8_t frameIndex) {
        auto& res = *final_swap_img_usage;
        Image& swapImage = *res.resource[frameIndex];
        imageBarrier.image = swapImage.image;
        if(swapImage.owningQueue == nullptr){
            imageBarrier.srcQueueFamilyIndex = presentQueue.family.index;
        }
        else{
            //EWE_ASSERT(swapImage.owningQueue != nullptr);
            imageBarrier.srcQueueFamilyIndex = swapImage.owningQueue->family.index;
        }
        
        imageBarrier.srcAccessMask = res.usage.accessMask;
        imageBarrier.srcStageMask = res.usage.stage;
        imageBarrier.oldLayout = res.usage.layout;
    }

    void PresentBridge::Execute(CommandBuffer& cmdBuf){
        EWE_ASSERT(!cmdBuf.debug_currentlyRendering);
        vkCmdPipelineBarrier2(cmdBuf, &dependencyInfo);
    }

}