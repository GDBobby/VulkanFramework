#include "EightWinds/Command/STC_Manager.h"

#include "EightWinds/RenderGraph/Resources.h"
#include "EightWinds/Backend/PipelineBarrier.h"
#include "EightWinds/RenderGraph/RenderGraph.h"
#include "EightWinds/Command/STC.h"

#include "EightWinds/Image.h"

#include "EightWinds/Reflect/Enum.h"

namespace EWE {


	Queue::Type STC_Manager::GetQueueType(Queue const& queue) const{
		if(queue == renderQueue){
			return Queue::Graphics;
		}
		else if (queue == computeQueue){
			return Queue::Compute;
		}
		else if (queue == transferQueue){
			return Queue::Transfer;
		}
		EWE_UNREACHABLE;
	}
	Queue& STC_Manager::GetQueueFromType(Queue::Type qType){
		switch (qType) {
			case Queue::Graphics: return renderQueue;
			case Queue::Compute: return computeQueue;
			case Queue::Transfer: return transferQueue;
			default: EWE_UNREACHABLE;
		}
		EWE_UNREACHABLE;
	}

	SingleTimeCommand* STC_Manager::GetSTC(Queue::Type requested_queue) {
		/*
		Queue::Type actualQueueType = requested_queue;
		switch (requested_queue) {
			case Queue::Transfer:
				if (transferQueue == renderQueue) {
					actualQueueType = Queue::Graphics;
				}
				else if (transferQueue == computeQueue) {
					//if the compute queue is also the transfer queue, it will behave the same as tranfer->render
					//if the destination queue is compute, it will be considered a 1 queue transfer
					actualQueueType = Queue::Compute; 
				}
				break;
			case Queue::Compute:
				if (computeQueue == renderQueue) {
					actualQueueType = Queue::Graphics;
				}
				break;
			default: break;
		}
		*/
		return new SingleTimeCommand(stc_command_pools[requested_queue].GetNext(), semaphores.GetNext());
	}

    STC_Manager::STC_Manager(LogicalDevice& logicalDevice, Queue& _renderQueue, Queue& _transferQueue, Queue& _computeQueue)
    : renderQueue{_renderQueue}, transferQueue{_transferQueue}, computeQueue{_computeQueue},
	renderCommandPools{logicalDevice, renderQueue, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT },
        stc_mutexes{},
        stc_command_pools{
            RingBuffer<CommandPool, 16>{logicalDevice, renderQueue, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT},
            RingBuffer<CommandPool, 16>{logicalDevice, computeQueue, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT},
            RingBuffer<CommandPool, 16>{logicalDevice, transferQueue, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT},
        },
		semaphores{logicalDevice}
    {
    }

    STC_Manager::~STC_Manager() {
		
    }

	SingleTimeCommand* STC_Manager::GetBeginSTC(){
		VkCommandBufferBeginInfo const cmdBeginInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
			.pInheritanceInfo = nullptr
		};

		SingleTimeCommand* stc = GetSTC(Queue::Transfer);
		stc->cmdBuf.Begin(cmdBeginInfo);

		return stc;
	}
	TimelineSemaphore* STC_Manager::EndAndSubmit(SingleTimeCommand& stc){
		stc.cmdBuf.End();
		VkCommandBufferSubmitInfo cmdInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
			.pNext = nullptr,
			.commandBuffer = stc.cmdBuf,
			.deviceMask = 0
		};

		//do i need to lock the mutex? probably
		semAcqMut.lock();
		TimelineSemaphore* ret = semaphores.GetNext();
		semAcqMut.unlock();
		ret->SetName("STC first");

		VkSemaphoreSubmitInfo semInfo = ret->GetSignalSubmitInfo(VK_PIPELINE_STAGE_2_TRANSFER_BIT);
		VkSubmitInfo2 submitInfo{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
			.pNext = nullptr,
			.waitSemaphoreInfoCount = 0,
			.commandBufferInfoCount = 1,
			.pCommandBufferInfos = &cmdInfo,
			.signalSemaphoreInfoCount = 1,
			.pSignalSemaphoreInfos = &semInfo
		};
		stc.cmdPool->queue.Submit2(1, &submitInfo, VK_NULL_HANDLE);
		return ret;
	}
    
	template<>
    void STC_Manager::AsyncTransfer(TransferContext<Image>& transferContext, Queue& rh_queue){
		AsyncTransfer(transferContext, GetQueueType(rh_queue));
	}

	template<>
	void STC_Manager::AsyncTransfer_Helper(TransferContext<Image>& transferContext, Queue::Type dstQueueType){
        Queue& rh_queue = GetQueueFromType(dstQueueType);

		SingleTimeCommand* stc = GetBeginSTC();

        VkImageLayout initial_layout = (transferContext.generatingMipMaps || dstQueueType == Queue::Type::Compute) ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		if (transferContext.final_usage.layout == VK_IMAGE_LAYOUT_GENERAL) {
			initial_layout = VK_IMAGE_LAYOUT_GENERAL;
		}
		const UsageData<Image> initial_usage{
			.stage = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
			.accessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
			.layout = initial_layout
		};

		transferContext.Acquire(stc->cmdBuf, transferQueue, initial_usage);
		transferContext.Stage(stc->cmdBuf, initial_usage);

		const auto owner_barriers = transferContext.ChangeOwnership(stc->cmdBuf, transferQueue, rh_queue, initial_usage);

		STC_Sub_Package<Image> stc_helper{
			.barriers{owner_barriers.Size()},
			.images{transferContext.images.Size()},
			.layout{transferContext.final_usage.layout},
			.dstQueue{&rh_queue}
		};
		for(std::size_t i = 0; i < transferContext.images.Size(); i++){
			stc_helper.images[i] = transferContext.images[i];
			stc_helper.barriers[i] = owner_barriers[i];
		}
		
		TimelineSemaphore* sem = EndAndSubmit(*stc);

		EWE_ASSERT(current_renderGraph != nullptr);
		current_renderGraph->ResourceOwnershipTransfer(stc_helper);

		TimelineSemaphore::RelinquishThreadControl(*sem);

		stc->cmdBuf.state = CommandBuffer::State::Invalid;
		transferContext.stagingBuffer.Free();

		semaphores.Return(sem);
		stc_command_pools[Queue::Transfer].Return(stc->cmdPool);
	}

	template<>
	void STC_Manager::Async_SingleQueueTransfer(TransferContext<Image>& transferContext, Queue::Type dstQueueType){

		Queue& queue = GetQueueFromType(dstQueueType);

		SingleTimeCommand* stc = GetBeginSTC();

        VkImageLayout initial_layout = (transferContext.generatingMipMaps || dstQueueType == Queue::Type::Compute) ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		if (transferContext.final_usage.layout == VK_IMAGE_LAYOUT_GENERAL) {
			initial_layout = VK_IMAGE_LAYOUT_GENERAL;
		}
		const UsageData<Image> initial_usage{
			.stage = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
			.accessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
			.layout = initial_layout
		};

		transferContext.Acquire(stc->cmdBuf, queue, initial_usage);
		transferContext.Stage(stc->cmdBuf, initial_usage);

		if (initial_usage.layout != transferContext.final_usage.layout) { //ownership and potentially layout transition
			transferContext.ChangeOwnership(stc->cmdBuf, queue, queue, initial_usage);
		}
		if (transferContext.generatingMipMaps) {
			EWE_ASSERT(stc->cmdPool->queue.family.SupportsGraphics());
		}
		
		TimelineSemaphore* sem = EndAndSubmit(*stc);

		TimelineSemaphore::RelinquishThreadControl(*sem);

		for(auto& img : transferContext.images){
			img->readyForUsage = true;
			img->data.layout = transferContext.final_usage.layout;
		}

		stc->cmdBuf.state = CommandBuffer::State::Invalid;
		transferContext.stagingBuffer.Free();

		semaphores.Return(sem);
		stc_command_pools[Queue::Transfer].Return(stc->cmdPool);
	}

    //fiber focused
	template<>
    void STC_Manager::AsyncTransfer(TransferContext<Image>& transferContext, Queue::Type dstQueueType){

		//EWE_ASSERT(!CheckMainThread());

#if EWE_DEBUG_BOOL
		NameCurrentThread("STCA");
#endif

        Queue& rh_queue = GetQueueFromType(dstQueueType);
		if (transferContext.generatingMipMaps) {
			EWE_ASSERT(rh_queue.family.SupportsGraphics());
		}

		//when im ready for signle queue transfers i can fix this up
        Queue& lh_queue = transferQueue;

		if(lh_queue != rh_queue){
			AsyncTransfer_Helper(transferContext, dstQueueType);
		}
		else{
			Async_SingleQueueTransfer(transferContext, dstQueueType);
		}
    }


	template<>
	void STC_Manager::Async_SingleQueueTransfer(TransferContext<Buffer>& transferContext, Queue::Type dstQueueType){
		Queue& queue = GetQueueFromType(dstQueueType);

		SingleTimeCommand* stc = GetBeginSTC();

        const UsageData<Buffer> initial_usage{
			.stage = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
			.accessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT
		};

		transferContext.Acquire(stc->cmdBuf, queue, initial_usage);
		transferContext.Stage(stc->cmdBuf);

		TimelineSemaphore* sem = EndAndSubmit(*stc);

		TimelineSemaphore::RelinquishThreadControl(*sem);

		for(auto& buf : transferContext.buffers) {
			buf->existsOnTheGPU = true;
			buf->owningQueue = &queue;
		}
		stc->cmdBuf.state = CommandBuffer::State::Invalid;
		transferContext.stagingBuffer.Free();
		
		semaphores.Return(sem);
		stc_command_pools[Queue::Transfer].Return(stc->cmdPool);
	}
	template<>
	void STC_Manager::AsyncTransfer_Helper(TransferContext<Buffer>& transferContext, Queue::Type dstQueueType){
		Queue& rh_queue = GetQueueFromType(dstQueueType);

		SingleTimeCommand* stc = GetBeginSTC();

		const UsageData<Buffer> initial_usage{
			.stage = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
			.accessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
		};

		transferContext.Acquire(stc->cmdBuf, transferQueue, initial_usage);
		transferContext.Stage(stc->cmdBuf);

		const auto owner_barriers = transferContext.ChangeOwnership(stc->cmdBuf, transferQueue, rh_queue, initial_usage);

		STC_Sub_Package<Buffer> stc_helper{
			.barriers{owner_barriers.Size()},
			.bufs{transferContext.buffers.Size()},
			.dstQueue = &rh_queue
		};	
		for(std::size_t i = 0; i < transferContext.buffers.Size(); i++){
			stc_helper.barriers[i] = owner_barriers[i];
			stc_helper.bufs[i] = transferContext.buffers[i];
		}

		TimelineSemaphore* sem = EndAndSubmit(*stc);

		EWE_ASSERT(current_renderGraph != nullptr);
		current_renderGraph->ResourceOwnershipTransfer(stc_helper);

		TimelineSemaphore::RelinquishThreadControl(*sem);

		stc->cmdBuf.state = CommandBuffer::State::Invalid;
		transferContext.stagingBuffer.Free();
		
		semaphores.Return(sem);
		stc_command_pools[Queue::Transfer].Return(stc->cmdPool);
	}
        


	template<>
	void STC_Manager::AsyncTransfer(TransferContext<Buffer>& transferContext, Queue& rh_queue){
		AsyncTransfer(transferContext, GetQueueType(rh_queue));
	}
	template<>
	void STC_Manager::AsyncTransfer(TransferContext<Buffer>& transferContext, Queue::Type dstQueueType){
		//EWE_ASSERT(!CheckMainThread(), "only works in a marl fiber rn\n");
		//i could assign the thread id to the STC_Context and check it within the rendergraph

#if EWE_DEBUG_BOOL
		NameCurrentThread("STCA");
#endif

        Queue& rh_queue = GetQueueFromType(dstQueueType);
		//when im ready for signle queue transfers i can fix this up
        Queue& lh_queue = transferQueue;

		if(lh_queue != rh_queue){
			AsyncTransfer_Helper(transferContext, dstQueueType);
		}
		else{
			Async_SingleQueueTransfer(transferContext, dstQueueType);
		}
	}


    template<>
    void STC_Manager::InlineTransfer(TransferContext<Image>& transferContext){

		SingleTimeCommand* stc = GetBeginSTC();

        VkImageLayout initial_layout = (transferContext.generatingMipMaps) ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		if (transferContext.final_usage.layout == VK_IMAGE_LAYOUT_GENERAL) {
			initial_layout = VK_IMAGE_LAYOUT_GENERAL;
		}
		const UsageData<Image> initial_usage{
			.stage = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
			.accessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
			.layout = initial_layout
		};

		transferContext.Acquire(stc->cmdBuf, renderQueue, initial_usage);
		transferContext.Stage(stc->cmdBuf, initial_usage);

		if (initial_usage.layout != transferContext.final_usage.layout) { //ownership and potentially layout transition
			transferContext.ChangeOwnership(stc->cmdBuf, renderQueue, renderQueue, initial_usage);
		}
		if (transferContext.generatingMipMaps) {
			EWE_ASSERT(stc->cmdPool->queue.family.SupportsGraphics());
		}
		
		TimelineSemaphore* sem = EndAndSubmit(*stc);

		TimelineSemaphore::RelinquishThreadControl(*sem);

		for(auto& img : transferContext.images){
			img->readyForUsage = true;
			img->data.layout = transferContext.final_usage.layout;
		}

		stc->cmdBuf.state = CommandBuffer::State::Invalid;
		transferContext.stagingBuffer.Free();

		semaphores.Return(sem);
		stc_command_pools[Queue::Transfer].Return(stc->cmdPool);
	}
    template<>
    void STC_Manager::InlineTransfer(TransferContext<Buffer>& transferContext){

		SingleTimeCommand* stc = GetBeginSTC();

        const UsageData<Buffer> initial_usage{
			.stage = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
			.accessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT
		};

		transferContext.Acquire(stc->cmdBuf, renderQueue, initial_usage);
		transferContext.Stage(stc->cmdBuf);

		TimelineSemaphore* sem = EndAndSubmit(*stc);

		TimelineSemaphore::RelinquishThreadControl(*sem);

		for(auto& buf : transferContext.buffers) {
			buf->existsOnTheGPU = true;
			buf->owningQueue = &renderQueue;
		}
		stc->cmdBuf.state = CommandBuffer::State::Invalid;
		transferContext.stagingBuffer.Free();
		
		semaphores.Return(sem);
		stc_command_pools[Queue::Transfer].Return(stc->cmdPool);
	}



    STC_Submitter::STC_Submitter(LogicalDevice& _logicalDevice, 
        Queue& _graphicsQueue, Queue& _computeQueue
    )
    : logicalDevice{_logicalDevice},
        graphicsQueue{_graphicsQueue}, computeQueue{_computeQueue},
        graphicsInfo{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO
        },
        computeInfo{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO
        }
    {
        graphics_task = [&](CommandBuffer& cmdBuf, uint8_t frameIndex){
            if(graphicsInfo.imageMemoryBarrierCount > 0 || graphicsInfo.bufferMemoryBarrierCount > 0){
                vkCmdPipelineBarrier2(cmdBuf, &graphicsInfo);
                graphics_image_barriers.clear();
                graphics_buffer_barriers.clear();
                return true;
            }
            return false;
        };
        compute_task = [&](CommandBuffer& cmdBuf, uint8_t frameIndex){
            if(computeInfo.imageMemoryBarrierCount > 0 || computeInfo.bufferMemoryBarrierCount > 0){
                vkCmdPipelineBarrier2(cmdBuf, &computeInfo);
                compute_image_barriers.clear();
                compute_buffer_barriers.clear();
                return true;
            }
            return false;
        };
    }

    bool STC_Submitter::CheckSize(Queue::Type qType) const {
        if(qType == Queue::Graphics){
            return graphicsInfo.imageMemoryBarrierCount > 0 || graphicsInfo.bufferMemoryBarrierCount > 0;
        }
        else if (qType == Queue::Compute){
            return computeInfo.imageMemoryBarrierCount > 0 || computeInfo.bufferMemoryBarrierCount > 0;
        }
        EWE_UNREACHABLE;
    }

    void STC_Submitter::CollectSTCs() {

        //this over-allocates, but that's ok.
        //not doing multiple allocations is better
        graphics_image_barriers.reserve(image_ownership.size());
        compute_image_barriers.reserve(image_ownership.size());
        graphics_buffer_barriers.reserve(buffer_ownership.size());
        compute_buffer_barriers.reserve(buffer_ownership.size());

        for(std::size_t i = 0; i < image_ownership.size(); i++){
            if(*image_ownership[i].dstQueue == graphicsQueue){
                for(auto& bar : image_ownership[i].barriers){
                    graphics_image_barriers.push_back(bar);
                }
            }
            else if(*image_ownership[i].dstQueue == computeQueue){
                for(auto& bar : image_ownership[i].barriers){
                    compute_image_barriers.push_back(bar);
                }
            }
            else{
                EWE_UNREACHABLE;
            }
        }
        for(std::size_t i = 0; i < buffer_ownership.size(); i++){
            if(*buffer_ownership[i].dstQueue == graphicsQueue){
                for(auto& bar : buffer_ownership[i].barriers){
                    graphics_buffer_barriers.push_back(bar);
                }
            }
            else if(*buffer_ownership[i].dstQueue == computeQueue){
                for(auto& bar : buffer_ownership[i].barriers){
                    compute_buffer_barriers.push_back(bar);
                }
            }
            else{
                EWE_UNREACHABLE;
            }
        }

        graphicsInfo.imageMemoryBarrierCount = static_cast<uint32_t>(graphics_image_barriers.size());
        graphicsInfo.bufferMemoryBarrierCount = static_cast<uint32_t>(graphics_buffer_barriers.size());
        computeInfo.imageMemoryBarrierCount = static_cast<uint32_t>(compute_image_barriers.size());
        computeInfo.bufferMemoryBarrierCount = static_cast<uint32_t>(compute_buffer_barriers.size());

        graphicsInfo.pImageMemoryBarriers = graphics_image_barriers.data();
        graphicsInfo.pBufferMemoryBarriers = graphics_buffer_barriers.data();
        computeInfo.pImageMemoryBarriers = compute_image_barriers.data();
        computeInfo.pBufferMemoryBarriers = compute_buffer_barriers.data();

        //VK_DEPENDENCY_QUEUE_FAMILY_OWNERSHIP_TRANSFER_USE_ALL_STAGES_BIT_KHR probably use this? idk 
        //https://docs.vulkan.org/spec/latest/chapters/synchronization.html#synchronization-queue-transfers search for the flag
        //VUID-vkCmdPipelineBarrier-maintenance8-10206 : If the maintenance8 feature is not enabled, dependencyFlags must not include ^ 
    }

    void STC_Submitter::Clear(){
        graphics_image_barriers.clear();
        graphics_buffer_barriers.clear();
        compute_image_barriers.clear();
        compute_buffer_barriers.clear();
        
        graphicsInfo.imageMemoryBarrierCount = 0;
        graphicsInfo.bufferMemoryBarrierCount = 0;
        computeInfo.imageMemoryBarrierCount = 0;
        computeInfo.bufferMemoryBarrierCount = 0;
    }
    void STC_Submitter::UpdateResources(){
        for(auto& img_owned : image_ownership){
            for(auto& img : img_owned.images){
                img->data.layout = img_owned.layout;
                img->readyForUsage = true;
                img->owningQueue = img_owned.dstQueue;
            }
        }
        for(auto& buf_owned : buffer_ownership){
            for(auto& buf : buf_owned.bufs){
                buf->owningQueue = buf_owned.dstQueue;
                buf->existsOnTheGPU = true;
            }
        }
        image_ownership.clear();
        buffer_ownership.clear();
    }

}//namespace EWE