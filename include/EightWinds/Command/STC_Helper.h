#pragma once

#include "EightWinds/VulkanHeader.h"

namespace EWE{

	struct StagingBuffer;
	struct CommandBuffer;
	struct LogicalDevice;
	struct Queue;
	struct Buffer;
	struct Image;

namespace Command_Helper{
	void Transition(LogicalDevice& logicalDevice, Queue& queue, VkDependencyInfo const& dependencyInfo, bool wait);
	void CopyBufferToImage(CommandBuffer& cmdBuf, StagingBuffer&, Image& img, VkBufferImageCopy const& region, VkImageLayout layout);
	void CopyBufferToImage(CommandBuffer& cmdBuf, StagingBuffer&, Image& img);

	void CopyBufferToBuffer(CommandBuffer& cmdBuf, StagingBuffer& lhs, Buffer& rhs, VkBufferCopy const& copyInfo);
	void CopyBufferToBuffer(CommandBuffer& cmdBuf, StagingBuffer& lhs, Buffer& rhs);
} //namespace Command_Helper
} //namespace EWE