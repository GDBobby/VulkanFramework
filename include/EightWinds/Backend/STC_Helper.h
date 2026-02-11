#pragma once

#include "EightWinds/VulkanHeader.h"

#include "EightWinds/LogicalDevice.h"
#include "EightWinds/Queue.h"

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
		
		template<typename Resource, bool Staging>
		struct SingleTime_Context{
			Queue& queue;
			Resource resource;
		};
		
		
		void BufferStaging(){
			
		}
	}
	
	/* using fibers, i dont need a callback, or a callback object
	struct TransferCommandPackage{
		LogicalDevice& logicalDevice;
		Queue& queue;
		CommandPool pool; //lives and dies here
		CommandBuffer cmdBuf;
		VkImageMemoryBarrier2 barrier;

		[[nodiscard]] explicit TransferCommandPackage(LogicalDevice& logicalDevice, Queue& queue);
	};
	*/
	
	template<typename Resource>
	struct TransferContext{
		//i want the context to describe the entire process, and to return data required for callbacks
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
		Resource<Image> resource; //final layout
		
		//a release and acquisition barrier are needed if transfering queues 
		//kind of a handshake between both queues
		Queue* transferQueue; //optional
		Queue& dstQueue; //i dont know if i can reflect upon this to see if it's going to be used for compute
		
		StagingBuffer* stagingBuffer; //also optional
		bool generatingMipMaps;
		
		VkBufferImageCopy image_region;
		
		void InitialBarrier(CommandBuffer& cmdBuf);
		
		void Commands(CommandBuffer& cmdBuf);
	};
	
	struct SingleQueueTransferContext_Image{
		Resource<Image> resource;
		Queue& dstQueue;
		StagingBuffer* stagingBuffer;
		bool generatingMipMaps;
		VkBufferImageCopy image_region;
		void Commands(CommandBuffer& cmdBuf);
	};
	
	struct AsyncTransferContext_Image{
		Resource<Image> resource;
		Queue& transferQueue;
		Queue& dstQueue;
		StagingBuffer* stagingBuffer;
		bool generatingMipMaps;
		VkBufferImageCopy image_region;
		void Commands(CommandBuffer& cmdBuf);
	};
	
	
}