#pragma once

#include "EightWinds/VulkanHeader.h"

#include "CommandPool.h"

namespace EWE{
    struct CommandBuffer {
        CommandPool& commandPool;

        vk::CommandBuffer cmdBuf;
        bool inUse;

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
        [[nodiscard]] CommandBuffer() : cmdBuf{ VK_NULL_HANDLE }, inUse{ false } {}
        operator vk::CommandBuffer() const { return cmdBuf; }
        operator vk::CommandBuffer*() { return &cmdBuf; }
#endif

        bool operator==(CommandBuffer const& other) const noexcept {
            return cmdBuf == other.cmdBuf;
        }
        void operator=(vk::CommandBuffer cmdBuf) noexcept {
            assert(this->cmdBuf == VK_NULL_HANDLE);
            this->cmdBuf = cmdBuf;
        }

        void Reset();
        void Begin();
        void BeginSingleTime();

        void BeginLabel(const char* name, float red, float green, float blue);
        void EndLabel();
    };
}