#include "EightWinds/CommandBuffer.h"

namespace EWE{
    void CommandBuffer::Reset() {
        //VkCommandBufferResetFlags flags = VkCommandBufferResetFlagBits::VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT;
        vk::CommandBufferResetFlags flags = vk::Flags<vk::CommandBufferResetFlagBits>(0);
        //EWE_VK(vkResetCommandBuffer, *this, flags);
        cmdBuf.reset(flags);
        inUse = false;

        assert(labelDepth == 0);
    }

        void CommandBuffer::Begin() {
        inUse = true;
        vk::CommandBufferBeginInfo beginInfo{};
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
        vk::CommandBufferBeginInfo beginInfo{};
        beginInfo.pNext = nullptr;
        beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
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
        vk::DebugUtilsLabelEXT utilLabel{};
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