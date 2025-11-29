#include "EightWinds/RenderGraph/GPUTask.h"

#include <cassert>

namespace EWE{

    PushTracker::PushTracker(GlobalPushConstant* ptr) noexcept
    : pushAddress{ptr}
    {
        for(uint8_t i = 0; i < GlobalPushConstant::buffer_count; i++){
            buffers[i] = nullptr;
        }
        for(uint8_t i = 0; i < GlobalPushConstant::texture_count; i++){
            textures[i] = nullptr;
        }
    }

    GPUTask::~GPUTask(){
        printf("need to destruct deferred pointers from CommandRecord, currently memory leak\n");
    }


    void GPUTask::UseBuffer(Buffer* buffer, uint32_t pushIndex, uint8_t slot, bool writes) noexcept{
        assert(slot < GlobalPushConstant::buffer_count);
         
        //first, check whats currently in the requested slot
        auto& desired_slot = pushTrackers[pushIndex].buffers[slot];
        if(desired_slot != nullptr) {
            //if the desired image is already in the requested slot, return
            if(desired_slot->resource == buffer){
                //maybe print a warning here, it would be a bit wasteful to call this every frame
                return;
            }
            //if another image is currently in the requested slot, remove it
            else {
                desired_slot->usageInCurrentTask--;
                if(desired_slot->usageInCurrentTask == 0){
                    delete desired_slot;
                }
            }
        }

        //second, check if the resource is already used in the task
        //if so increment it and copy the pointer over
        for(auto& used : usedBuffers){
            if(used->resource == buffer) { //this is a pointer comparison
                used->usageInCurrentTask++;
                desired_slot = used;
                break;
            }
        }

        //third, if it still hasnt been found, create and add it
        desired_slot = new Resource<Buffer>(buffer, writes, 1);
        usedBuffers.push_back(desired_slot);
    }
    void GPUTask::UseImage(ImageInfo* imgInfo, uint32_t pushIndex, uint8_t slot, bool writes) noexcept{
        assert(slot < (GlobalPushConstant::texture_count + GlobalPushConstant::buffer_count));
        if(slot >= GlobalPushConstant::buffer_count){
            slot -= GlobalPushConstant::buffer_count;
        }
        
        //first, check whats currently in the requested slot
        auto& desired_slot = pushTrackers[pushIndex].textures[slot];
        if(desired_slot != nullptr) {
            //if the desired image is already in the requested slot, return
            if(desired_slot->resource == imgInfo){
                //maybe print a warning here, it would be a bit wasteful to call this every frame
                return;
            }
            //if another image is currently in the requested slot, remove it
            else {
                desired_slot->usageInCurrentTask--;
                if(desired_slot->usageInCurrentTask == 0){
                    delete desired_slot;
                }
            }
        }

        //second, check if the resource is already used in the task
        //if so increment it and copy the pointer over
        for(auto& used : usedImages){
            if(used->resource == imgInfo) { //this is a pointer comparison
                used->usageInCurrentTask++;
                desired_slot = used;
                break;
            }
        }

        //third, if it still hasnt been found, create and add it
        desired_slot = new Resource<ImageInfo>(imgInfo, writes, 1);
        usedImages.push_back(desired_slot);
    }
    /*
    void GPUTask::BeginRender(CommandBuffer& cmdBuf) const noexcept{

        assert(!(renderInfo.colorAttachmentInfo.resolveImageView != VK_NULL_HANDLE) && (renderInfo.colorAttachmentInfo.resolveMode == VK_RESOLVE_MODE_NONE));
        assert(!(renderInfo.colorAttachmentInfo.resolveMode != VK_RESOLVE_MODE_NONE && renderInfo.colorAttachmentInfo.resolveImageView == VK_NULL_HANDLE));

        vkCmdBeginRendering(cmdBuf, &renderInfo.renderingInfo);
    }
    */
} //namespace EWE