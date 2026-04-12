#include "EightWinds/RenderGraph/GPUTask.h"

#include "EightWinds/CommandBuffer.h"
#include "EightWinds/Command/Record.h"

#include "EightWinds/RenderGraph/Resources.h"

namespace EWE{

    GPUTask::GPUTask(std::string_view _name, LogicalDevice& _logicalDevice, Queue& _queue)
    : name{ _name },
        logicalDevice{ _logicalDevice },
        queue{ _queue },
        pkgRecord{nullptr}
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

    GPUTask::GPUTask(std::string_view _name, LogicalDevice& _logicalDevice, Command::PackageRecord& record, bool compile)
    : GPUTask{_name, _logicalDevice, *record.queue}
    {
        if(compile){
            paramPool.emplace(record.Compile());
        }
        pkgRecord = &record;
    }


    GPUTask::~GPUTask(){
#if EWE_DEBUG_BOOL
        Logger::Print<Logger::Error>("need to destruct deferred pointers from CommandRecord, currently memory leak\n");
#endif
    }
    bool GPUTask::Execute(CommandBuffer& cmdBuf, uint8_t frameIndex) {
        EWE_ASSERT(cmdBuf.commandPool.queue == queue);
        EWE_ASSERT(!(commandExecutor.has_value() && paramPool.has_value()), "only 1 can have a value");
        if(commandExecutor.has_value()){
            commandExecutor->Execute(cmdBuf, frameIndex);
            return true;
        }
        else if(paramPool.has_value()){
            Command::ExecuteParamPool(paramPool.value(), logicalDevice, cmdBuf, frameIndex);
            return paramPool->instructions.size() > 0;
        }
        else if(pkgRecord != nullptr){
            bool did_something = false;
            for(auto& rec : pkgRecord->packages){
                Command::ExecuteParamPool(rec->paramPool, logicalDevice, cmdBuf, frameIndex);
                did_something |= rec->paramPool.instructions.size() > 0;
            }
            return did_something;
        }
        else if(external_workload != nullptr){
            return external_workload(cmdBuf, frameIndex);
        }
        else{
            EWE_UNREACHABLE;
        }
        return false;
    }

    void GPUTask::GenerateWorkload() {

        workload = [&](CommandBuffer& cmdBuf, uint8_t frameIndex) {
            prefix.Execute(cmdBuf, frameIndex);
            bool ret = Execute(cmdBuf, frameIndex);
            suffix.Execute(cmdBuf, frameIndex);
            return ret;
        };
    }
} //namespace EWE