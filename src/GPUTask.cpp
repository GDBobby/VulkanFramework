#include "EightWinds/RenderGraph/GPUTask.h"

#include "EightWinds/CommandBuffer.h"
#include "EightWinds/RenderGraph/Command/Record.h"

#include "EightWinds/RenderGraph/Resources.h"

namespace EWE{

    GPUTask::GPUTask(std::string_view name, LogicalDevice& logicalDevice, Queue& queue, Command::Record& cmdRecord)
    : name{ name },
        logicalDevice{logicalDevice},
        queue{queue},
        commandExecutor{std::in_place, logicalDevice, cmdRecord}
    {
    }


    GPUTask::GPUTask(std::string_view name, LogicalDevice& logicalDevice, Queue& queue)
    : name{ name },
        logicalDevice{ logicalDevice },
        queue{ queue }
    {
    }

    GPUTask::~GPUTask(){
#if EWE_DEBUG_BOOL
        printf("need to destruct deferred pointers from CommandRecord, currently memory leak\n");
#endif
    }
    void GPUTask::Execute(CommandBuffer& cmdBuf, uint8_t frameIndex) {
        EWE_ASSERT(cmdBuf.commandPool.queue == queue);
        commandExecutor->Execute(cmdBuf, frameIndex);
    }

    /*
    void GPUTask::SetRenderInfo() {
        EWE_ASSERT(renderTracker != nullptr);
        renderTracker->compact.Expand(&renderTracker->vk_data);
        
        bool hasBeginRender = false;
        //there's only going to be one BeginRender, and this search will only be performed once on construction
        for (auto& inst : commandExecutor.instructions) {
            if (inst.type == CommandInstruction::Type::BeginRender) {
                VkRenderingInfo** tempAddr = reinterpret_cast<VkRenderingInfo**>(commandExecutor.paramPool.data() + inst.paramOffset);
                *tempAddr = &renderTracker->vk_data.renderingInfo;
                inst.paramOffset;
                hasBeginRender = true;
                break;
            }
        }
        EWE_ASSERT(hasBeginRender);
    }
    void GPUTask::UpdateFrameIndex(uint8_t frameIndex) {
        renderTracker->compact.Update(&renderTracker->vk_data, frameIndex);
    }
    */


    void GPUTask::GenerateWorkload() {
        if (external_workload != nullptr) {
            GenerateExternalWorkload();
            return;
        }

        workload = [&](CommandBuffer& cmdBuf, uint8_t frameIndex) {
            prefix.Execute(cmdBuf, frameIndex);
            Execute(cmdBuf, frameIndex);
            suffix.Execute(cmdBuf, frameIndex);
            return true;
        };
    }
    void GPUTask::GenerateExternalWorkload() {


#if EWE_DEBUG_BOOL
        //EWE_ASSERT(external_workload != nullptr);
        if (commandExecutor->record.records.size() > 0) {
            printf("warning : ignoring the command executor\n");
        }
#endif
        workload = [&](CommandBuffer& cmdBuf, uint8_t frameIndex) {
            prefix.Execute(cmdBuf, frameIndex);
            external_workload(cmdBuf, frameIndex);
            suffix.Execute(cmdBuf, frameIndex);
            return true;
        };
    }
} //namespace EWE