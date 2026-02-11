#pragma once

#include "EightWinds/VulkanHeader.h"

#include "EightWinds/Backend/RenderInfo.h"
#include "EightWinds/RenderGraph/Command/Execute.h"
#include "EightWinds/RenderGraph/Resources.h"
#include "EightWinds/RenderGraph/TaskAffix.h"

#include "EightWinds/CommandPool.h"

#include "EightWinds/GlobalPushConstant.h"


#include <span>

//equivalent a renderpass subpass?

namespace EWE{
    namespace Command {
        struct Record;
    }
    //the id of this task is its address
    //GPUTask is intended to be used in a single thread.
    //if its desired to multi-thread within a single task, sync needs to be external
    //i need to be able to insert barriers
    struct GPUTask{
        std::string name;
        LogicalDevice& logicalDevice;
        //i think i define a command pool here, or at least a queue
        Queue& queue;

        Command::Executor commandExecutor;

        [[nodiscard]] explicit GPUTask(std::string_view name, LogicalDevice& logicalDevice, Queue& queue, Command::Record& cmdRecord);
        [[nodiscard]] explicit GPUTask(std::string_view name, LogicalDevice& logicalDevice, Queue& queue);
        ~GPUTask();
        GPUTask(GPUTask const&) = delete;
        GPUTask& operator=(GPUTask const&) = delete;
        GPUTask(GPUTask&&) = delete;
        GPUTask& operator=(GPUTask&&) = delete;

        //i need another system wrapping GPUTask to handle how the command buffers are dealt with
        void Execute(CommandBuffer& cmdBuf, uint8_t frameIndex);

        std::function<bool(CommandBuffer& cmdBuf, uint8_t frameIndex)> workload = nullptr;
        std::function<bool(CommandBuffer& cmdBuf, uint8_t frameIndex)> external_workload = nullptr;
        std::function<bool(uint8_t frameIndex)> independent_workload = nullptr;
        
        TaskResourceUsage resources;

        TaskAffix prefix{};
        TaskAffix suffix{};
        void GenerateWorkload();
        void GenerateExternalWorkload();
    };


}