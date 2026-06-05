#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/RenderGraph/Resources.h"

#include <vector>

namespace EWE {
	struct Queue;
	struct StagingBuffer;

namespace Barrier{
	VkImageMemoryBarrier2 Acquire_Image(Queue& dstQueue, Image& img, UsageData<Image> const& usage);
	VkBufferMemoryBarrier2 Acquire_Buffer(Queue& dstQueue, Buffer& buf, UsageData<Buffer> const& usage);
	
	VkImageMemoryBarrier2 Transition_Image(Queue& srcQueue, Image& img, Queue& dstQueue, UsageData<Image> const& lh_usage, UsageData<Image> const& rh_usage);
	VkBufferMemoryBarrier2 Transition_Buffer(Queue& srcQueue, Buffer& buf, Queue& dstQueue, UsageData<Buffer> const& lh_usage, UsageData<Buffer> const& rh_usage);
	

} //namespace Barrier
} //namespace EWE
