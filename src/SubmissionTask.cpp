#include "EightWinds/RenderGraph/SubmissionTask.h"

namespace EWE{

    void TaskSubmissionWorkload::GenerateBridges(){
        
        bridges.clear();
        bridges.reserve(ordered_gpuTasks.size());

        bridges.emplace_back(*ordered_gpuTasks.front());
        for(std::size_t i = 1; i < ordered_gpuTasks.size(); i++){
            bridges.emplace_back(*ordered_gpuTasks[i - 1], *ordered_gpuTasks[i]);
        }
    }

    bool TaskSubmissionWorkload::Execute(CommandBuffer& cmdBuf) {
        TaskBridge lhBridge{*ordered_gpuTasks.front()};
        lhBridge.Execute(cmdBuf);

        if(ordered_gpuTasks.size() > 1){
            for(std::size_t i = 0; i < (ordered_gpuTasks.size() - 1); i++) {
                ordered_gpuTasks[i]->Execute(cmdBuf);
                TaskBridge tempBridge{*ordered_gpuTasks[i], *ordered_gpuTasks[i]};
                tempBridge.RecreateBarriers();
                tempBridge.Execute(cmdBuf);
            }

            ordered_gpuTasks.back()->Execute(cmdBuf);
        }
        return ordered_gpuTasks.size() > 0;
    }

    SubmissionTask::SubmissionTask(LogicalDevice& logicalDevice, Queue& queue, bool signals, std::string_view name)
        : logicalDevice{logicalDevice}, queue{queue},
        cmdPool{logicalDevice, queue, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT},
        cmdBuffers{ cmdPool.AllocateCommandsPerFlight(VK_COMMAND_BUFFER_LEVEL_PRIMARY) },
        signal{signals},
        signal_semaphores{logicalDevice, false},
        full_workload{nullptr},
        external_workload{nullptr},
        name{name}
    {
#if EWE_DEBUG_NAMING
        for (uint8_t i = 0; i < EWE::max_frames_in_flight; i++) {
            std::string debug_name = std::string(name) + "submission task [" + std::to_string(i) + ']';
            signal_semaphores[i].SetName(debug_name);
            cmdBuffers[i].SetDebugName(debug_name);
        }
#endif
    }

    bool SubmissionTask::Execute(uint8_t frameIndex) {

        bool ret = true;
        if(full_workload){

            //beginInfo
            VkCommandBufferBeginInfo cmdBeginInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .pNext = nullptr,
                .flags = 0,
                .pInheritanceInfo = nullptr  
            };
            cmdBuffers[frameIndex].Reset();
            cmdBuffers[frameIndex].Begin(cmdBeginInfo);

#if EWE_DEBUG_NAMING
            VkDebugUtilsLabelEXT labelUtil{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
                .pNext = nullptr,
                .pLabelName = name.c_str(),
                .color = {0.f, 0.f, 0.f, 1.f}
            };
            logicalDevice.BeginLabel(cmdBuffers[frameIndex], &labelUtil);
#endif

            ret = full_workload(cmdBuffers[frameIndex]);

#if EWE_DEBUG_NAMING
            logicalDevice.EndLabel(cmdBuffers[frameIndex]);
#endif

            cmdBuffers[frameIndex].End();

            cmdBuffers[frameIndex].state = CommandBuffer::State::Pending;
        }
        else if(external_workload){
            ret = external_workload(submitInfo[frameIndex], frameIndex);
        }
        else{
            assert(false && "no active workload"); 
        }

        return ret;
    }



    SubmissionBridge::SubmissionBridge(std::span<SubmissionTask*> lhs, SubmissionTask* rhs)
        : lhs{lhs}, rhs{rhs}
    {
        for (uint8_t i = 0; i < max_frames_in_flight; i++) {
            for (auto& lh : lhs) {
                assert(lh->signal); //i dont know if i want to force this or not
                rhs->submitInfo[i].WaitOnPrevious(lh->submitInfo[i]);
            }
        }
        
    }
    
}