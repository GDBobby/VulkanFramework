#include "EightWinds/CommandBuffer.h"

#include <cassert>

namespace EWE{

    PFN_vkCmdBeginDebugUtilsLabelEXT pfnBeginLabel;
    PFN_vkCmdEndDebugUtilsLabelEXT pfnEndLabel;

    static bool InitLabelFunctions(VkDevice device) noexcept {
        pfnBeginLabel = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetDeviceProcAddr(device, "vkCmdBeginDebugUtilsLabelEXT");
        pfnEndLabel = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetDeviceProcAddr(device, "vkCmdEndDebugUtilsLabelEXT");

        return pfnBeginLabel != nullptr && pfnEndLabel != nullptr;
    }

    
    CommandBuffer::CommandBuffer(CommandPool& commandPool, VkCommandBuffer cmdBuf) noexcept
        : commandPool{commandPool}
    {
        VkCommandBufferBeginInfo cmdBufBeginInfo{};
        cmdBufBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cmdBufBeginInfo.pNext = nullptr;
        cmdBufBeginInfo.pInheritanceInfo = nullptr;//this is some subpass type, i need to research this
        //cmdBufBeginInfo.flags = flags //<--- this needs to be handled
        EWE_VK(vkBeginCommandBuffer, cmdBuf, &cmdBufBeginInfo);
    }
    CommandBuffer::~CommandBuffer(){
        //vkEndCommandBuffer(cmdBuf); //i dont like this. it also needs to be submitted

        assert(commandPool.allocatedBuffers > 0);
        commandPool.allocatedBuffers--;
    }
/*
    CommandBuffer& CommandBuffer::operator=(VkCommandBuffer cmdBuf) noexcept;
        void operator=(VkCommandBuffer cmdBuf) noexcept {
        assert(this->cmdBuf == VK_NULL_HANDLE);
        this->cmdBuf = cmdBuf;

        return *this;
    }
*/
    void CommandBuffer::Reset() noexcept {
        assert(commandPool.flags & VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT && inUse);
        EWE_VK(vkResetCommandBuffer, cmdBuf, resetFlags);
        inUse = false;

        assert(labelDepth == 0);
    }

        void CommandBuffer::Begin() noexcept {
        inUse = true;
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.pNext = nullptr;
#if COMMAND_BUFFER_TRACING
        if (usageTracking.size() > 2) {
            usageTracking.pop();
        }
        usageTracking.push({});
#endif
        vkBeginCommandBuffer(cmdBuf, &beginInfo);
    }
    void CommandBuffer::BeginSingleTime() noexcept {
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
        EWE_VK(vkBeginCommandBuffer, cmdBuf, &beginInfo);
    }

#define DEBUG_NAMING 1

    void CommandBuffer::BeginLabel(const char* name, float red, float green, float blue) noexcept {
#if DEBUG_NAMING
        VkDebugUtilsLabelEXT utilLabel{};
        utilLabel.pNext = nullptr;
        utilLabel.color[0] = red;
        utilLabel.color[1] = green;
        utilLabel.color[2] = blue;
        utilLabel.color[3] = 1.f;
        utilLabel.pLabelName = name;
        //cmdBuf.beginDebugUtilsLabelEXT(utilLabel);
        pfnBeginLabel(cmdBuf, &utilLabel);

#endif
        ++labelDepth;

    }
    void CommandBuffer::EndLabel() noexcept {
#if DEBUG_NAMING
        //cmdBuf.endDebugUtilsLabelEXT();
        pfnEndLabel(cmdBuf);
#endif
        --labelDepth;
    }
}