#include "EightWinds/RenderGraph/GPUTask.h"

namespace EWE{
    void GPUTask::BeginRender(CommandBuffer& cmdBuf) const noexcept{

        assert(!(renderInfo.colorAttachmentInfo.resolveImageView != VK_NULL_HANDLE) && (renderInfo.colorAttachmentInfo.resolveMode == VK_RESOLVE_MODE_NONE));
        assert(!(renderInfo.colorAttachmentInfo.resolveMode != VK_RESOLVE_MODE_NONE && renderInfo.colorAttachmentInfo.resolveImageView == VK_NULL_HANDLE));

        vkCmdBeginRendering(cmdBuf, &renderInfo.renderingInfo);
    }
} //namespace EWE