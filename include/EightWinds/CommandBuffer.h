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
        bool inUse = false;

        VkCommandBufferResetFlags resetFlags = 0;

        int8_t labelDepth = 0;
#if COMMAND_BUFFER_TRACING
        struct Tracking {
            std::string funcName;
            std::stacktrace stackTrace;
            Tracking(std::string const& funcName) : funcName{ funcName } { printf("need to set up stack trace here\n"); }
        };
        std::queue<std::vector<Tracking>> usageTracking;

        CommandBuffer() : cmdBuf{ VK_NULL_HANDLE }, inUse{ false }, usageTracking{} {}
#else
        [[nodiscard]] explicit CommandBuffer(CommandPool& commandPool, VkCommandBuffer cmdBuf) noexcept;
        ~CommandBuffer();

        operator VkCommandBuffer() const { return cmdBuf; }
        operator VkCommandBuffer*() { return &cmdBuf; }
#endif

        bool operator==(CommandBuffer const& other) const noexcept { return cmdBuf == other.cmdBuf;}
        //CommandBuffer& operator=(VkCommandBuffer cmdBuf) noexcept;

        void Reset() noexcept;
        void Begin() noexcept;
        void BeginSingleTime() noexcept;
        //submit single time? im removing synchub for sure

        void BeginLabel(const char* name, float red, float green, float blue) noexcept;
        void EndLabel() noexcept;

        static bool InitLabelFunctions() noexcept;
    };

    struct DetailedCommandBuffer{
        VkCommandBuffer cmdBuf;


        void BindPipeline(PipelineID pipeID);

        //i think i want a descirptor set that contains the details for buffers and images contained
        void BindDescriptor(VkDescriptorSet set);

        void BindVertexBuffer(VkBuffer vert);
        void BindIndexBuffer(VkBuffer index);

        void Push(void* push);

        //this shouldnt be used directly
        void BeginRender(VkRenderingInfo const& renderInfo);
        void EndRender();

        void PipelineBarrier(PipelineBarrier const& pipeBarrier);

        void BeginLabel(const char* name, float red, float green, float blue) noexcept;
        void EndLabel() noexcept;

        void SetDynamicState(VkDynamicState dynState, void* ambiguousData);

        template<typename F, typename... Args>
        auto SetDynamicState(F&& f, Args&&... args)
        requires std::is_invocable_v<F, VkCommandBuffer, Args...>
        {
            return std::forward<F>(f)(cmdBuf, std::forward<Args>(args)...);
        }
    };
}