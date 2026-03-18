#include "EightWinds/CommandBuffer.h"

namespace EWE{

    
    CommandBuffer::CommandBuffer(CommandPool& commandPool, VkCommandBuffer cmdBuf) noexcept
        : commandPool{commandPool},
        cmdBuf{cmdBuf}
    {
    }
    CommandBuffer::CommandBuffer(CommandBuffer&& moveSrc) noexcept
        : commandPool{ moveSrc.commandPool },
        cmdBuf{ moveSrc.cmdBuf }
    {
        moveSrc.cmdBuf = VK_NULL_HANDLE;
    }

    CommandBuffer::~CommandBuffer(){
        //vkEndCommandBuffer(cmdBuf); //i dont like this. it also needs to be submitted

        EWE_ASSERT(commandPool.allocatedBuffers > 0);
        EWE_ASSERT(state == CommandBuffer::State::Invalid || state == CommandBuffer::State::Initial
               || state == CommandBuffer::State::Pending); //this one is just mean time, until i manage to automate up pending -> invalid transition
    }

    void CommandBuffer::Reset() {
        /*
        ideally State::Pending would be transitioned to invalid with a fence callback
        and this would assert that the state is currently invalid
        UNTIL i get that setup, this is going to require state be in present
        */
        EWE_ASSERT(commandPool.flags & VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
#if EWE_DEBUG_BOOL
        if (state == State::Pending) {
#else
        if(state == State::Initial){
#endif
            state = State::Initial;
            EWE_VK(vkResetCommandBuffer, cmdBuf, resetFlags);

            EWE_ASSERT(labelDepth == 0);
        }
    }

    void CommandBuffer::End() {
        EWE_ASSERT(state == State::Recording);
        state = State::Executable;
        EWE_VK(vkEndCommandBuffer, cmdBuf);
    }

    void CommandBuffer::Begin(VkCommandBufferBeginInfo const& beginInfo) {
        EWE_ASSERT(state == State::Initial);
        state = State::Recording;
#if COMMAND_BUFFER_TRACING
        if (usageTracking.size() > 2) {
            usageTracking.pop();
        }
        usageTracking.push({});
#endif
        EWE_VK(vkBeginCommandBuffer, cmdBuf, &beginInfo);
    }

#if EWE_DEBUG_NAMING
    void CommandBuffer::SetDebugName(std::string_view name) {
        debugName = name;
        commandPool.logicalDevice.SetObjectName(cmdBuf, VK_OBJECT_TYPE_COMMAND_BUFFER, name);
    }
#endif
}