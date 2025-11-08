#include "EightWinds/CommandBuffer.h"

namespace EWE{
    void CommandBuffer::Reset() {
        assert(commandPool.flags & VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT && inUse);
        vkResetCommandBuffer(cmdBuf, flags);
        inUse = false;

        assert(labelDepth == 0);
    }

        void CommandBuffer::Begin() {
        inUse = true;
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.pNext = nullptr;
#if COMMAND_BUFFER_TRACING
        if (usageTracking.size() > 2) {
            usageTracking.pop();
        }
        usageTracking.push({});
#endif
        cmdBuf.begin(beginInfo);
    }
    void CommandBuffer::BeginSingleTime() {
        inUse = true;
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.pNext = nullptr;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
#if COMMAND_BUFFER_TRACING
        if (usageTracking.size() > 2) {
            usageTracking.pop();
        }
        usageTracking.push({});
#endif
        cmdBuf.begin(beginInfo);
    }

#define DEBUG_NAMING 1

    void CommandBuffer::BeginLabel(const char* name, float red, float green, float blue) {
#if DEBUG_NAMING
        VkDebugUtilsLabelEXT utilLabel{};
        utilLabel.pNext = nullptr;
        utilLabel.color[0] = red;
        utilLabel.color[1] = green;
        utilLabel.color[2] = blue;
        utilLabel.color[3] = 1.f;
        utilLabel.pLabelName = name;
        cmdBuf.beginDebugUtilsLabelEXT(utilLabel);

#endif
        ++labelDepth;

    }
    void CommandBuffer::EndLabel() {
#if DEBUG_NAMING
        cmdBuf.endDebugUtilsLabelEXT();
#endif
        --labelDepth;
    }
}