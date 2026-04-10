#pragma once

#include "EightWinds/VulkanHeader.h"

#include "EightWinds/Backend/RenderInfo.h"
#include "EightWinds/Command/Execute.h"
#include "EightWinds/RenderGraph/Resources.h"
#include "EightWinds/RenderGraph/TaskAffix.h"

#include "EightWinds/CommandPool.h"

#include "EightWinds/Command/PackageRecord.h"

#include <optional>
#include <span>
#include <string_view>

namespace EWE{
    namespace Command {
        struct Record;
    }
    //the id of this task is its address
    //GPUTask is intended to be used in a single thread.
    //if its desired to multi-thread within a single task, sync needs to be external
    struct GPUTask{
        std::string name;
        LogicalDevice& logicalDevice;
        //i think i define a command pool here, or at least a queue
        Queue& queue;

        std::optional<Command::Executor> commandExecutor;
        std::optional<Command::ParamPool> paramPool;
        Command::PackageRecord* pkgRecord; //just for viewing in reconstruction

        [[nodiscard]] explicit GPUTask(std::string_view name, LogicalDevice& logicalDevice, Queue& queue);
        [[nodiscard]] explicit GPUTask(std::string_view name, LogicalDevice& logicalDevice, Queue& queue, Command::Record& cmdRecord);
        [[nodiscard]] explicit GPUTask(std::string_view name, LogicalDevice& logicalDevice, Queue& queue, Command::ParamPool& pp);
        [[nodiscard]] explicit GPUTask(std::string_view name, LogicalDevice& logicalDevice, Command::PackageRecord& record);
        ~GPUTask();
        GPUTask(GPUTask const&) = delete;
        GPUTask(GPUTask&&) = delete;
        GPUTask& operator=(GPUTask const&) = delete;
        GPUTask& operator=(GPUTask&&) = delete;

        //i need another system wrapping GPUTask to handle how the command buffers are dealt with
        bool Execute(CommandBuffer& cmdBuf, uint8_t frameIndex);

        std::function<bool(CommandBuffer& cmdBuf, uint8_t frameIndex)> workload = nullptr;
        std::function<bool(CommandBuffer& cmdBuf, uint8_t frameIndex)> external_workload = nullptr; //will be packed between prefix and suffix into workload
        
        TaskResourceUsage resources;

        TaskAffix prefix{};
        TaskAffix suffix{};
        void GenerateWorkload();
    };


}