#include "EightWinds/RenderGraph/GPUTask.h"

#include "EightWinds/CommandBuffer.h"
#include "EightWinds/Command/Record.h"

#include "EightWinds/RenderGraph/Resources.h"

namespace EWE{

    GPUTask::GPUTask(std::string_view _name, LogicalDevice& _logicalDevice, Queue& _queue)
    : name{ _name },
        logicalDevice{ _logicalDevice },
        queue{ _queue }
    {
    }
    GPUTask::GPUTask(std::string_view _name, LogicalDevice& _logicalDevice, Queue& _queue, Command::Record& cmdRecord)
    : GPUTask{_name, _logicalDevice, _queue}//,
        //commandExecutor{std::in_place, logicalDevice, cmdRecord}
    {
        commandExecutor.emplace(logicalDevice, cmdRecord);
    }

    GPUTask::GPUTask(std::string_view _name, LogicalDevice& _logicalDevice, Queue& _queue, Command::ParamPool& pp)
    : GPUTask{_name, _logicalDevice, _queue}
    {
        paramPool.emplace(pp);

    }

    GPUTask::GPUTask(std::string_view _name, LogicalDevice& _logicalDevice, Command::PackageRecord const& record)
    : GPUTask{_name, _logicalDevice, *record.queue}
    {
        paramPool.emplace(record.Compile());
    }


    GPUTask::~GPUTask(){
#if EWE_DEBUG_BOOL
        Logger::Print<Logger::Error>("need to destruct deferred pointers from CommandRecord, currently memory leak\n");
#endif
    }
    void GPUTask::Execute(CommandBuffer& cmdBuf, uint8_t frameIndex) {
        EWE_ASSERT(cmdBuf.commandPool.queue == queue);
        EWE_ASSERT(commandExecutor.has_value() != paramPool.has_value(), "its assumed one or the other has a value rn");
        if(commandExecutor.has_value()){
            commandExecutor->Execute(cmdBuf, frameIndex);
        }
        else{
            Command::ExecuteParamPool(paramPool.value(), logicalDevice, cmdBuf, frameIndex);
        }
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