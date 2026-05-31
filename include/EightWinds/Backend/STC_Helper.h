#pragma once

#include "EightWinds/VulkanHeader.h"

#include "EightWinds/LogicalDevice.h"
#include "EightWinds/Queue.h"

#include "EightWinds/RenderGraph/Resources.h"
#include "EightWinds/Image.h"

#include "EightWinds/Backend/StagingBuffer.h"
#include "EightWinds/CommandPool.h"
#include "EightWinds/CommandBuffer.h"

namespace EWE{
	struct Buffer;
	struct Image;
	namespace Command_Helper{
		
		void Transition(LogicalDevice& logicalDevice, Queue& queue, VkDependencyInfo const& dependencyInfo, bool wait);
		void CopyBufferToImage(CommandBuffer& cmdBuf, VkBuffer vkbuffer, Image& img, VkBufferImageCopy const& region, VkImageLayout layout);
		void CopyBufferToImage(CommandBuffer& cmdBuf, VkBuffer vkbuffer, Image& img);

		void CopyBufferToBuffer(CommandBuffer& cmdBuf, StagingBuffer& lhs, Buffer& rhs, VkBufferCopy const& copyInfo);
		void CopyBufferToBuffer(CommandBuffer& cmdBuf, StagingBuffer& lhs, Buffer& rhs);
		
		template<typename Resource, bool Staging>
		struct SingleTime_Context{
			Queue& queue;
			Resource resource;
		};
	}
	
	template<typename Resource>
	struct TransferContext{
		//i want the context to describe the entire process, and to return data required for callbacks
	};
	template<>
	struct TransferContext<Buffer>{
		Resource<Buffer> resource;
		StagingBuffer* stagingBuffer;
		VkBufferCopy buffer_region;
	};
	
	template<>
	struct TransferContext<Image>{
		Resource<Image> resource; //set the dstlayout here
		StagingBuffer* stagingBuffer;
		bool generatingMipMaps;
		VkBufferImageCopy image_region;
	};
}