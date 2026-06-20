#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/RenderGraph/Resources.h"

#include "EightWinds/Image.h"
#include "EightWinds/Buffer.h"

#include <vector>

namespace EWE {
	struct Queue;
	struct StagingBuffer;

namespace Barrier{

	template<ResourceType Resource>
	//the return type is Resource::Barrier, which will be VkImageMemoryBarrier2 and VkBufferMemoryBarrier2
	Resource::Barrier Acquire(Queue& dstQueue, Resource& res, UsageData<Resource> const& usage, AcquireType acqType = AcquireType::None);
	
	template<ResourceType Resource>
	Resource::Barrier Transition(Queue& srcQueue, Resource& res, Queue& dstQueue, UsageData<Resource> const& lh_usage, UsageData<Resource> const& rh_usage);

	template<> Image::Barrier Acquire(Queue& dstQueue, Image& img, UsageData<Image> const& usage, AcquireType acqType);
	template<> Buffer::Barrier Acquire(Queue& dstQueue, Buffer& buf, UsageData<Buffer> const& usage, AcquireType acqType);
	
	template<> Image::Barrier Transition(Queue& srcQueue, Image& img, Queue& dstQueue, UsageData<Image> const& lh_usage, UsageData<Image> const& rh_usage);
	template<> Buffer::Barrier Transition(Queue& srcQueue, Buffer& buf, Queue& dstQueue, UsageData<Buffer> const& lh_usage, UsageData<Buffer> const& rh_usage);
	

	

} //namespace Barrier
} //namespace EWE
