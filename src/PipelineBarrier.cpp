#include "EightWinds/Backend/PipelineBarrier.h"

#include "EightWinds/Queue.h"
#include "EightWinds/Buffer.h"
#include "EightWinds/Image.h"

#include <iterator>

namespace EWE {
namespace Barrier {
	
	VkImageMemoryBarrier2 Acquire_Image(Queue& dstQueue, Image& img, UsageData<Image> const& usage){
		//making them both equal is the same as VK_QUEUE_FAMILY_IGNORED
		const uint32_t srcQueueFamilyIndex = img.owningQueue ? img.owningQueue->FamilyIndex() : dstQueue.FamilyIndex(); 
		return VkImageMemoryBarrier2{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.pNext = nullptr,
			.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
			.srcAccessMask = VK_ACCESS_2_NONE,
			.dstStageMask = usage.stage,
			.dstAccessMask = usage.accessMask, 
			.oldLayout = img.data.layout,
			.newLayout = usage.layout,
			.srcQueueFamilyIndex = srcQueueFamilyIndex,
			.dstQueueFamilyIndex = dstQueue.FamilyIndex(),
			.image = img.image,
			.subresourceRange = VkImageSubresourceRange{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = img.data.mipLevels,
				.baseArrayLayer = 0,
				.layerCount = img.data.arrayLayers
			}
		};
	}
	VkBufferMemoryBarrier2 Acquire_Buffer(Queue& dstQueue, Buffer& buf, UsageData<Buffer> const& usage){
		const uint32_t srcQueueFamilyIndex = buf.owningQueue ? buf.owningQueue->FamilyIndex() : dstQueue.FamilyIndex();
		return VkBufferMemoryBarrier2{
			.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
			.pNext = nullptr,
			.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
			.srcAccessMask = VK_ACCESS_2_NONE,
			.dstStageMask = usage.stage,
			.dstAccessMask = usage.accessMask,
			.srcQueueFamilyIndex = srcQueueFamilyIndex, //if this is nullptr, this field needs to be equal to dstQueueFamilyIndex
			.dstQueueFamilyIndex = dstQueue.FamilyIndex(),
			.buffer = buf.buffer_info.buffer,
			.offset = 0,
			.size = buf.bufferSize
		};
	}
	
	VkImageMemoryBarrier2 Transition_Image(Queue& srcQueue, Image& img, Queue& dstQueue, UsageData<Image> const& lh_usage, UsageData<Image> const& rh_usage) {
		return VkImageMemoryBarrier2{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.pNext = nullptr,
			.srcStageMask = lh_usage.stage,
			.srcAccessMask = lh_usage.accessMask,
			.dstStageMask = rh_usage.stage,
			.dstAccessMask = rh_usage.accessMask,
			.oldLayout = lh_usage.layout, 
			.newLayout = rh_usage.layout,
			.srcQueueFamilyIndex = srcQueue.FamilyIndex(),
			.dstQueueFamilyIndex = dstQueue.FamilyIndex(),
			.image = img.image,
			.subresourceRange = VkImageSubresourceRange{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = img.data.mipLevels,
				.baseArrayLayer = 0,
				.layerCount = img.data.arrayLayers
			}
		};
	}
	VkBufferMemoryBarrier2 Transition_Buffer(Queue& srcQueue, Buffer& buf, Queue& dstQueue, UsageData<Buffer> const& lh_usage, UsageData<Buffer> const& rh_usage){
		return VkBufferMemoryBarrier2{
			.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
			.pNext = nullptr,
			.srcStageMask = lh_usage.stage,
			.srcAccessMask = lh_usage.accessMask,
			.dstStageMask = rh_usage.stage,
			.dstAccessMask = rh_usage.accessMask,
			.srcQueueFamilyIndex = srcQueue.FamilyIndex(),
			.dstQueueFamilyIndex = dstQueue.FamilyIndex(),
			.buffer = buf.buffer_info.buffer,
			.offset = 0,
			.size = buf.bufferSize
		};
	}
	
}//namespace Barrier
} //namespace EWE