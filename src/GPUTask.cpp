#include "EightWinds/RenderGraph/GPUTask.h"

#include "EightWinds/CommandBuffer.h"

#include "EightWinds/RenderGraph/Resources.h"

namespace EWE{

    GPUTask::GPUTask(std::filesystem::path const& _name, LogicalDevice& _logicalDevice, Queue& _queue)
    : name{ _name },
        logicalDevice{ _logicalDevice },
        queue{ _queue },
        pkgRecord{nullptr}
    {
    }

    GPUTask::GPUTask(std::filesystem::path const& _name, LogicalDevice& _logicalDevice, Queue& _queue, Command::ParamPool& pp)
    : GPUTask{_name, _logicalDevice, _queue}
    {
        paramPool.emplace(pp);

    }

    GPUTask::GPUTask(std::filesystem::path const& _name, LogicalDevice& _logicalDevice, Command::PackageRecord& record, bool compile)
    : GPUTask{_name, _logicalDevice, *record.queue}
    {
        if(compile){
            paramPool.emplace(record.Compile());
        }
        pkgRecord = &record;
    }


    GPUTask::~GPUTask(){}

    bool GPUTask::Execute(CommandBuffer& cmdBuf, uint8_t frameIndex) {
        EWE_ASSERT(cmdBuf.commandPool.queue == queue);
        if(paramPool.has_value()){
            Command::ExecuteParamPool(logicalDevice, cmdBuf, paramPool.value(), frameIndex);
            return paramPool->instructions.size() > 0;
        }
        else if(pkgRecord != nullptr){
            bool did_something = false;
            for(auto& rec : pkgRecord->packages){
                Command::ExecuteParamPool(logicalDevice, cmdBuf, rec->paramPool, frameIndex);
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