#include "EightWinds/RenderGraph/STCs.h"

namespace EWE{

    STCManagement::STCManagement(LogicalDevice& _logicalDevice, 
        Queue& _graphicsQueue, Queue& _computeQueue, 
        SubmissionTask& _graphics_stc_task, SubmissionTask& _compute_stc_task
    )
    : logicalDevice{_logicalDevice},
        graphicsQueue{_graphicsQueue}, computeQueue{_computeQueue},
        graphics_stc_task{_graphics_stc_task}, compute_stc_task{_compute_stc_task},
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

    bool STCManagement::CheckSize(Queue::Type qType) const {
        if(qType == Queue::Graphics){
            return graphicsInfo.imageMemoryBarrierCount > 0 || graphicsInfo.bufferMemoryBarrierCount > 0;
        }
        else if (qType == Queue::Compute){
            return computeInfo.imageMemoryBarrierCount > 0 || computeInfo.bufferMemoryBarrierCount > 0;
        }
        EWE_UNREACHABLE;
    }

    void STCManagement::CollectSTCs() {

        //this over-allocates, but that's ok.
        //not doing multiple allocations is better
        graphics_image_barriers.reserve(image_ownership.size());
        compute_image_barriers.reserve(image_ownership.size());
        graphics_buffer_barriers.reserve(buffer_ownership.size());
        compute_buffer_barriers.reserve(buffer_ownership.size());

        for(std::size_t i = 0; i < image_ownership.size(); i++){
            if(*image_ownership[i].dstQueue == graphicsQueue){
                graphics_image_barriers.push_back(image_ownership[i].barrier);
            }
            else if(*image_ownership[i].dstQueue == computeQueue){
                compute_image_barriers.push_back(image_ownership[i].barrier);
            }
            else{
                EWE_UNREACHABLE;
            }
        }
        for(std::size_t i = 0; i < buffer_ownership.size(); i++){
            if(*buffer_ownership[i].dstQueue == graphicsQueue){
                graphics_buffer_barriers.push_back(buffer_ownership[i].barrier);
            }
            else if(*buffer_ownership[i].dstQueue == computeQueue){
                compute_buffer_barriers.push_back(buffer_ownership[i].barrier);
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

    void STCManagement::Clear(){
        graphics_image_barriers.clear();
        graphics_buffer_barriers.clear();
        compute_image_barriers.clear();
        compute_buffer_barriers.clear();
        
        graphicsInfo.imageMemoryBarrierCount = 0;
        graphicsInfo.bufferMemoryBarrierCount = 0;
        computeInfo.imageMemoryBarrierCount = 0;
        computeInfo.bufferMemoryBarrierCount = 0;
    }
    void STCManagement::UpdateResources(){
        for(auto& img : image_ownership){
            img.res.resource[0]->data.layout = img.res.usage.layout;
            img.res.resource[0]->readyForUsage = true;
            img.res.resource[0]->owningQueue = img.dstQueue;
        }
        image_ownership.clear();
        buffer_ownership.clear();
    }
} //namespace EWE