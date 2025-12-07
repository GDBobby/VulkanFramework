#include "EightWinds/RenderGraph/GPUTask.h"

#include "EightWinds/Command/CommandBuffer.h"
#include "EightWinds/Command/Record.h"

#include <cassert>

namespace EWE{

    PushTracker::PushTracker(GlobalPushConstant* ptr) noexcept
    : pushAddress{ptr}
    {
        for(uint8_t i = 0; i < GlobalPushConstant::buffer_count; i++){
            buffers[i].resource = nullptr;
        }
        for(uint8_t i = 0; i < GlobalPushConstant::texture_count; i++){
            textures[i].resource = nullptr;
        }
    }

    GPUTask::GPUTask(LogicalDevice& logicalDevice, Queue& queue, CommandRecord& cmdRecord, std::string_view name) 
        : logicalDevice{logicalDevice}, 
        queue{queue},
        commandExecutor{logicalDevice},
        name{name}
    {
        assert(!cmdRecord.hasBeenCompiled);
        //cmdRecord.Optimize(); <--- EVENTUALLY
        const uint64_t full_data_size = cmdRecord.records.back().paramOffset + CommandInstruction::GetParamSize(cmdRecord.records.back().type);

        commandExecutor.instructions = cmdRecord.records;
        commandExecutor.paramPool.resize(full_data_size);
        const std::size_t param_pool_address = reinterpret_cast<std::size_t>(commandExecutor.paramPool.data());
        cmdRecord.FixDeferred(param_pool_address);
        for(auto& push_off : cmdRecord.push_offsets){
            std::size_t temp_addr = reinterpret_cast<std::size_t>(push_off);
            pushTrackers.emplace_back(reinterpret_cast<GlobalPushConstant*>(temp_addr + param_pool_address));
        }
        uint64_t blitIndex = 0;
        for (auto const& inst : cmdRecord.records) {
            if (inst.type == CommandInstruction::Type::BeginRender) {
                renderTracker = new RenderTracker();
            }
            if(inst.type == CommandInstruction::Type::Blit) {
                auto& blitBack = blitTrackers.emplace_back();
                blitBack.dstImage.resource = nullptr;
                blitBack.srcImage.resource = nullptr;
            }
        }

        //all validations will be here
        //theres some non-validation stuff here, like collapsing empty branches
        //maybe split out optimization into a different loop
#if EWE_DEBUG_BOOL
        assert(cmdRecord.ValidateInstructions());
#endif
       cmdRecord.hasBeenCompiled = true;
    

    }

    GPUTask::~GPUTask(){
#if EWE_DEBUG_BOOL
        printf("need to destruct deferred pointers from CommandRecord, currently memory leak\n");
#endif
        if (renderTracker!= nullptr) {
            delete renderTracker;
        }
    }
    void GPUTask::Execute(CommandBuffer& cmdBuf) {
        assert(cmdBuf.commandPool.queue == queue);
        commandExecutor.Execute(cmdBuf);
    }

    /*
    template<typename T>
    void ReplaceResource(std::vector<Resource<T>*>& used, Resource<T>*& oldRes, T* repRes, bool writes) {
        if (oldRes->resource == repRes) {
            return;
        }
        //this is a removal, but it can be put here for simplicity
        if (repRes == nullptr) {

            return;
        }

        //remove old resource
        assert(oldRes->usageInCurrentTask >= 1);
        oldRes->usageInCurrentTask--;
        if (oldRes->usageInCurrentTask == 0) {
            for (auto iter = used.begin(); iter != used.end(); iter++) {
                if ((*iter) == oldRes) {
                    used.erase(iter);
                }
            }
            delete oldRes;
        }

        //check if its already used in the task
        bool foundInUse = false;
        for (auto& us : used) {
            if (us->resource == repRes) {
                us->usageInCurrentTask++;
                oldRes = us;
                foundInUse = true;
                break;
            }
        }
        //set the new dst image
        if (!foundInUse) {
            //its written to, so the second constructor param is true
            used.push_back(new Resource<T>(repRes, true));
            oldRes = used.back();
        }
    }
    */
    
    void GPUTask::DefineBlitUsage(uint16_t blitIndex, Image* srcImage, Image* dstImage) noexcept {
        assert(blitIndex < blitTrackers.size());
        auto& tracker = blitTrackers[blitIndex];

        //SHOULD i guarantee they're in transfer src/dst optimal?
        if (srcImage != nullptr) {
            tracker.srcImage.resource = srcImage;
            tracker.srcImage.usage.accessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
            tracker.srcImage.usage.stage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            tracker.srcImage.layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        }
        else {
            tracker.srcImage.resource = nullptr;
        }

        if (dstImage != nullptr) {
            tracker.dstImage.resource = dstImage;
            tracker.dstImage.usage.accessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            tracker.dstImage.usage.stage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            tracker.dstImage.layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        }
        else {
            tracker.dstImage.resource = nullptr;
        }
    }


    void GPUTask::PushBuffer(Buffer* buffer, uint32_t pushIndex, uint8_t slot, ResourceUsageData const& usageData) noexcept{
        assert(slot < GlobalPushConstant::buffer_count);
         
        //first, check whats currently in the requested slot
        auto& desired_slot = pushTrackers[pushIndex].buffers[slot];
        //ReplaceResource(usedBuffers, desired_slot, buffer);
        if (buffer != nullptr) {
            pushTrackers[pushIndex].pushAddress->buffer_addr[slot] = buffer->deviceAddress;
            desired_slot.resource = buffer;
            desired_slot.usage = usageData;
        }
        else {
            pushTrackers[pushIndex].pushAddress->buffer_addr[slot] = 0;
            desired_slot.resource = nullptr;
        }
    }
    void GPUTask::PushImage(Image* img, uint32_t pushIndex, uint8_t slot, ResourceUsageData const& usageData, VkImageLayout layout) noexcept{
        assert(slot < (GlobalPushConstant::texture_count + GlobalPushConstant::buffer_count));
        if(slot >= GlobalPushConstant::buffer_count){
            slot -= GlobalPushConstant::buffer_count;
        }
        auto& desired_slot = pushTrackers[pushIndex].textures[slot];

        //ReplaceResource(usedImages, desired_slot, img);
        if (img != nullptr) {
#if EWE_DEBUG_BOOL
            printf("set up the image texture index\n");
#endif
            assert(img->texture_index >= 0);
            pushTrackers[pushIndex].pushAddress->texture_indices[slot] = img->texture_index;
            desired_slot.resource = img;
            desired_slot.usage = usageData;
            desired_slot.layout = layout;
        }
        else {
            pushTrackers[pushIndex].pushAddress->texture_indices[slot] = -1;
            desired_slot.resource = nullptr;
        }
    }

    void GPUTask::SetRenderInfo() {
        assert(renderTracker != nullptr);
        renderTracker->compact.Expand(&renderTracker->vk_data);
        
        bool hasBeginRender = false;
        for (auto& inst : commandExecutor.instructions) {
            if (inst.type == CommandInstruction::Type::BeginRender) {
                VkRenderingInfo** tempAddr = reinterpret_cast<VkRenderingInfo**>(commandExecutor.paramPool.data() + inst.paramOffset);
                *tempAddr = &renderTracker->vk_data.renderingInfo;
                inst.paramOffset;
                hasBeginRender = true;
                break;
            }
        }
        assert(hasBeginRender);
    }
    void GPUTask::UpdateFrameIndex(uint8_t frameIndex) {
        renderTracker->compact.Update(&renderTracker->vk_data, frameIndex);
    }
} //namespace EWE