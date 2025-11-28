#include "EightWinds/RenderGraph/GPUTask.h"

#include "EightWinds/GlobalPushConstant.h"

#include <cassert>

namespace EWE{

    GPUTask::~GPUTask(){
        printf("need to destruct deferred pointers from CommandRecord, currently memory leak\n");
    }


    void GPUTask::UseBuffer(Buffer* buffer, GlobalPushConstant* pushPtr, uint8_t slot, bool writes) noexcept{
        assert(slot < GlobalPushConstant::buffer_count);
        pushptr->buffer_addresses[slot] = buffer->deviceAddress;
    }
    void GPUTask::UseImage(ImageInfo* imgInfo, GlobalPushConstant* pushPtr, uint8_t slot, bool writes) noexcept{
        assert(slot < (GlobalPushConstant::texture_count + GlobalPushConstant::buffer_count));
        if(slot >= GlobalPushConstant::buffer_count){
            slot -= GlobalPushConstant::buffer_count;
        }
        pushPtr->texture_indices[slot] = imgInfo->descriptorIndex;
    }
    /*
    void GPUTask::BeginRender(CommandBuffer& cmdBuf) const noexcept{

        assert(!(renderInfo.colorAttachmentInfo.resolveImageView != VK_NULL_HANDLE) && (renderInfo.colorAttachmentInfo.resolveMode == VK_RESOLVE_MODE_NONE));
        assert(!(renderInfo.colorAttachmentInfo.resolveMode != VK_RESOLVE_MODE_NONE && renderInfo.colorAttachmentInfo.resolveImageView == VK_NULL_HANDLE));

        vkCmdBeginRendering(cmdBuf, &renderInfo.renderingInfo);
    }
    */
} //namespace EWE