#pragma once

#include "EightWinds/VulkanHeader.h"

#include "EightWinds/LogicalDevice.h"
#include "EightWinds/Queue.h"

#include "EightWinds/Backend/StagingBuffer.h"

namespace EWE{
	namespace Command_Helper{
		struct Buffer;
		struct Image;
		
		void Transition(LogicalDevice& logicalDevice, Queue& queue, VkDependencyInfo const& dependencyInfo, bool wait);
		void CopyBufferToImage(CommandBuffer& cmdBuf, VkBuffer buffer, Image& img, VkBufferImageCopy const& region);
		void CopyBufferToImage(CommandBuffer& cmdBuf, VkBuffer buffer, Image& img);
		
		template<typename Resource, bool Staging>
		struct SingleTime_Context{
			Queue& queue;
			Resource resource;
			
		};
		
	}
	
	struct TransferCommandPackage{
		CommandPool pool; //lives and dies here
		CommandBuffer cmdBuf;
		VkImageMemoryBarrier2 barrier;
	};
	
	template<typename Resource>
	struct TransferContext{
		//the transfer is going to be on a fiber which could be moved across threads
		//so we'll create a command pool that last for the lifetime of the fiber
	};
	template<>
	struct TransferContext<Buffer>{
		Buffer& buffer;
		//if lhQueue == renderQueue, the logic will be changed a bit.
		
		//a release and acquisition barrier are needed if transfering queues 
		//kind of a handshake between both queues
		Queue* transferQueue; //optional
		Queue& dstQueue;
		
		StagingBuffer* stagingBuffer; //also optional
	};
	
	template<>
	struct TransferContext<Image>{
		Image& buffer;
		//if lhQueue == renderQueue, the logic will be changed a bit.
		
		//a release and acquisition barrier are needed if transfering queues 
		//kind of a handshake between both queues
		Queue* transferQueue; //optional
		Queue& dstQueue; //i dont know if i can reflect upon this to see if it's going to be used for compute
		VkImageLayout dstLayout;
		
		StagingBuffer* stagingBuffer; //also optional
		
		
		TransferCommandPackage* Commands();
	};
}