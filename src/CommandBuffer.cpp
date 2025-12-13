#include "EightWinds/Command/CommandBuffer.h"

#include <cassert>

namespace EWE{

    
    CommandBuffer::CommandBuffer(CommandPool& commandPool, VkCommandBuffer cmdBuf) noexcept
        : commandPool{commandPool},
        cmdBuf{cmdBuf}
    {
    }
    CommandBuffer::~CommandBuffer(){
        //vkEndCommandBuffer(cmdBuf); //i dont like this. it also needs to be submitted

        assert(commandPool.allocatedBuffers > 0);
        commandPool.allocatedBuffers--;
    }

    void CommandBuffer::Reset() {
        /*
        ideally State::Pending would be transitioned to invalid with a fence callback
        and this would assert that the state is currently invalid
        UNTIL i get that setup, this is going to require state be in present
        */
        assert(commandPool.flags & VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
        if (state == State::Pending) {
            EWE_VK(vkResetCommandBuffer, cmdBuf, resetFlags);

            assert(labelDepth == 0);
            state = State::Initial;
        }
    }

    void CommandBuffer::End() {
#if EWE_DEBUG_BOOL
        assert(state == State::Recording);
        state = State::Executable;
#endif
        EWE_VK(vkEndCommandBuffer, cmdBuf);
    }

    void CommandBuffer::Begin(VkCommandBufferBeginInfo const& beginInfo) {
#if EWE_DEBUG_BOOL
        assert(state == State::Initial);
        state = State::Recording;
#endif
#if COMMAND_BUFFER_TRACING
        if (usageTracking.size() > 2) {
            usageTracking.pop();
        }
        usageTracking.push({});
#endif
        EWE_VK(vkBeginCommandBuffer, cmdBuf, &beginInfo);
    }
}