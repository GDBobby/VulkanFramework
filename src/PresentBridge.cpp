#include "EightWinds/RenderGraph/PresentBridge.h"

#include "EightWinds/Command/CommandBuffer.h"

namespace EWE{
    PresentBridge::PresentBridge(LogicalDevice& logicalDevice, Queue& presentQueue) noexcept
    : logicalDevice{logicalDevice},
        presentQueue{presentQueue},
        name{"present bridge"}
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


		dependencyInfo.pMemoryBarriers = nullptr;
		dependencyInfo.pImageMemoryBarriers = &imageBarrier;
		dependencyInfo.pBufferMemoryBarriers = nullptr;
    }
    
    void PresentBridge::UpdateSrcData(Queue* lhsQueue, Resource<Image>* resource) {
        imageBarrier.image = *resource->image;
        imageBarrier.srcQueueFamilyIndex = lhsQueue->family.index;
        imageBarrier.srcAccessMask = resource->usage.accessMask; //idk what to put here
        imageBarrier.srcStageMask = resource->usage.stage;
        imageBarrier.oldLayout = resource->usage.layout;
        return;
    }

    void PresentBridge::SetSubresource(VkImageSubresourceRange const& subresource) {
        imageBarrier.subresourceRange = subresource;
    }

    void PresentBridge::Execute(CommandBuffer& cmdBuf){
#if EWE_DEBUG_BOOL
        assert(!cmdBuf.debug_currentlyRendering);
#endif
        vkCmdPipelineBarrier2(cmdBuf, &dependencyInfo);
    }

}