#include "EightWinds/RenderGraph/GPUTask.h"

#include "EightWinds/CommandBuffer.h"
#include "EightWinds/RenderGraph/Command/Record.h"

#include "EightWinds/RenderGraph/Resources.h"

#include <cassert>

namespace EWE{

    GPUTask::GPUTask(LogicalDevice& logicalDevice, Queue& queue, CommandRecord& cmdRecord, std::string_view name) 
        : logicalDevice{logicalDevice}, 
        queue{queue},
        commandExecutor{logicalDevice},
        name{name}
    {
        assert(!cmdRecord.hasBeenCompiled);
        //cmdRecord.Optimize(); <--- EVENTUALLY
        const uint64_t full_data_size = cmdRecord.records.back().paramOffset + CommandInstruction::GetParamSize(cmdRecord.records.back().type);

        commandExecutor.instructions = cmdRecord.records;
        PerFlight<std::size_t> param_pool_addresses{};
        for (uint8_t i = 0; i < max_frames_in_flight; i++) {
            commandExecutor.paramPool[i].resize(full_data_size);
            param_pool_addresses[i] = reinterpret_cast<std::size_t>(commandExecutor.paramPool[i].data());
        }
        cmdRecord.FixDeferred(param_pool_addresses);
        //for(auto& push_off : cmdRecord.push_offsets){
            //std::size_t temp_addr = reinterpret_cast<std::size_t>(push_off);
            //pushTrackers.emplace_back(reinterpret_cast<GlobalPushConstant*>(temp_addr + param_pool_address));
        //}

        //all validations will be here
        //theres some non-validation stuff here, like collapsing empty branches
        //maybe split out optimization into a different loop
#if EWE_DEBUG_BOOL
        assert(cmdRecord.ValidateInstructions());
#endif
       cmdRecord.hasBeenCompiled = true;
    

    }

    GPUTask::~GPUTask(){
#if EWE_DEBUG_BOOL
        printf("need to destruct deferred pointers from CommandRecord, currently memory leak\n");
#endif
    }
    void GPUTask::Execute(CommandBuffer& cmdBuf, uint8_t frameIndex) {
        assert(cmdBuf.commandPool.queue == queue);
        commandExecutor.Execute(cmdBuf, frameIndex);
    }

    /*
    void GPUTask::SetRenderInfo() {
        assert(renderTracker != nullptr);
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
        assert(hasBeginRender);
    }
    void GPUTask::UpdateFrameIndex(uint8_t frameIndex) {
        renderTracker->compact.Update(&renderTracker->vk_data, frameIndex);
    }
    */


    void GPUTask::GenerateWorkload()
    {
        const bool hasPrefix = !prefix.Empty();
        const bool hasSuffix = !suffix.Empty();

        if (hasPrefix && hasSuffix) {
            workload = [&](CommandBuffer& cmdBuf, uint8_t frameIndex) {
                prefix.Execute(cmdBuf, frameIndex);
                Execute(cmdBuf, frameIndex);
                suffix.Execute(cmdBuf, frameIndex);
            };
        }
        else if (hasPrefix) {
            workload = [&](CommandBuffer& cmdBuf, uint8_t frameIndex) {
                prefix.Execute(cmdBuf, frameIndex);
                Execute(cmdBuf, frameIndex);
            };
        }
        else if (hasSuffix) {
            workload = [&](CommandBuffer& cmdBuf, uint8_t frameIndex) {
                Execute(cmdBuf, frameIndex);
                suffix.Execute(cmdBuf, frameIndex);
            };
        }
        else {
            workload = [&](CommandBuffer& cmdBuf, uint8_t frameIndex) {
                Execute(cmdBuf, frameIndex);
                };
        }
    }
} //namespace EWE