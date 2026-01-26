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
    struct CommandRecord;
    
    //the id of this task is its address
    //GPUTask is intended to be used in a single thread.
    //if its desired to multi-thread within a single task, sync needs to be external
    //i need to be able to insert barriers
    struct GPUTask{
        LogicalDevice& logicalDevice;
        //i think i define a command pool here, or at least a queue
        Queue& queue;

        std::string name;
        CommandExecutor commandExecutor;

        [[nodiscard]] explicit GPUTask(LogicalDevice& logicalDevice, Queue& queue, CommandRecord& cmdRecord, std::string_view name);
        [[nodiscard]] explicit GPUTask(LogicalDevice& logicalDevice, Queue& queue, std::string_view name);
        ~GPUTask();
        GPUTask(GPUTask const&) = delete;
        GPUTask& operator=(GPUTask const&) = delete;
        GPUTask(GPUTask&&) = delete;
        GPUTask& operator=(GPUTask&&) = delete;

        //i need another system wrapping GPUTask to handle how the command buffers are dealt with
        void Execute(CommandBuffer& cmdBuf, uint8_t frameIndex);

        std::function<void(CommandBuffer& cmdBuf, uint8_t frameIndex)> workload = nullptr;
        
        TaskResourceUsage resources;

        TaskPrefix prefix{logicalDevice, queue};
        TaskSuffix suffix{logicalDevice, queue};
        void GenerateWorkload();
        void GenerateExternalWorkload(std::function<void(CommandBuffer& cmdBuf, uint8_t frameIndex)> external_workload);
    };


}