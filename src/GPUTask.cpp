#include "EightWinds/RenderGraph/GPUTask.h"

#include "EightWinds/CommandBuffer.h"
#include "EightWinds/RenderGraph/Command/Record.h"

#include "EightWinds/RenderGraph/Resources.h"

namespace EWE{

    GPUTask::GPUTask(std::string_view _name, LogicalDevice& _logicalDevice, Queue& _queue, Command::Record& cmdRecord)
    : name{ _name },
        logicalDevice{_logicalDevice},
        queue{_queue},
        commandExecutor{std::in_place, logicalDevice, cmdRecord}
    {
    }


    GPUTask::GPUTask(std::string_view _name, LogicalDevice& _logicalDevice, Queue& _queue)
    : name{ _name },
        logicalDevice{ _logicalDevice },
        queue{ _queue }
    {
    }

    GPUTask::~GPUTask(){
#if EWE_DEBUG_BOOL
        Logger::Print<Logger::Error>("need to destruct deferred pointers from CommandRecord, currently memory leak\n");
#endif
    }
    void GPUTask::Execute(CommandBuffer& cmdBuf, uint8_t frameIndex) {
        EWE_ASSERT(cmdBuf.commandPool.queue == queue);
        commandExecutor->Execute(cmdBuf, frameIndex);
    }

    void GPUTask::GenerateWorkload() {

        workload = [&](CommandBuffer& cmdBuf, uint8_t frameIndex) {
            prefix.Execute(cmdBuf, frameIndex);
            Execute(cmdBuf, frameIndex);
            suffix.Execute(cmdBuf, frameIndex);
            return true;
        };
    }
    void GPUTask::GenerateExternalWorkload(std::function<bool(CommandBuffer& cmdBuf, uint8_t frameIndex)> external_workload) {


        if (commandExecutor->record.records.size() > 0) {
            Logger::Print<Logger::Warning>("ignoring a non-empty command executor\n");
        }
        workload = [&](CommandBuffer& cmdBuf, uint8_t frameIndex) {
            prefix.Execute(cmdBuf, frameIndex);
            bool ret = external_workload(cmdBuf, frameIndex);
            suffix.Execute(cmdBuf, frameIndex);
            return ret;
        };
    }
} //namespace EWE