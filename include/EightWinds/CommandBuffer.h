#pragma once

#include "EightWinds/VulkanHeader.h"

#include "EightWinds/CommandPool.h"

namespace EWE{

    //this is gonna be a handle to an in-use command buffer.
    //it'll be returned by CommandPool
    //ESSENTIALLY, it can be considered a view
    // ill have to come back to that, but i definitely need to interact with the command pool more than i was

    struct CommandBuffer {
        CommandPool& commandPool;

        VkCommandBuffer cmdBuf;
        enum class State {
            Initial,
            Recording,
            Executable,
            Pending,
            Invalid,
        };
        //the state needs to be explicitly set to pending
        //and then explicitly set to Invalid at the moment
        //but ideally, a fence callback would indicate when it transitions from pending to invalid
        State state = State::Initial;

        VkCommandBufferResetFlags resetFlags = 0;

#if EWE_DEBUG_BOOL
        bool debug_currentlyRendering = false;
#endif

        int8_t labelDepth = 0;
#if COMMAND_BUFFER_TRACING
        struct Tracking {
            std::string funcName;
            std::stacktrace stackTrace;
            Tracking(std::string const& funcName) : funcName{ funcName } { printf("need to set up stack trace here\n"); }
        };
        std::queue<std::vector<Tracking>> usageTracking;

        CommandBuffer(CommandPool& commandPool) : commandPool{ commandPool }, cmdBuf { VK_NULL_HANDLE }, inUse{ false }, usageTracking{} {}
#else
        [[nodiscard]] explicit CommandBuffer(CommandPool& commandPool, VkCommandBuffer cmdBuf) noexcept;
        CommandBuffer(CommandBuffer const& copySrc) = delete;
        CommandBuffer& operator=(CommandBuffer const& copySrc) = delete;
        CommandBuffer(CommandBuffer&& moveSrc) noexcept;
        ~CommandBuffer();

        operator VkCommandBuffer() const { return cmdBuf; }
        operator VkCommandBuffer*() { return &cmdBuf; }
#endif

        bool operator==(CommandBuffer const& other) const noexcept { return cmdBuf == other.cmdBuf;}
        //CommandBuffer& operator=(VkCommandBuffer cmdBuf) noexcept;

        void Reset();
        void Begin(VkCommandBufferBeginInfo const& beginInfo);
        void End();

#if EWE_DEBUG_NAMING
        std::string debugName;
        void SetDebugName(std::string_view name);
#endif
    };
}