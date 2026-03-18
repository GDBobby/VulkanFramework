#include "EightWinds/RenderGraph/SubmissionTask.h"

namespace EWE{

    /*
    void TaskSubmissionWorkload::GenerateBridges(){
        
        bridges.clear();
        bridges.reserve(ordered_gpuTasks.size());

        bridges.emplace_back(*ordered_gpuTasks.front());
        for(std::size_t i = 1; i < ordered_gpuTasks.size(); i++){
            bridges.emplace_back(*ordered_gpuTasks[i - 1], *ordered_gpuTasks[i]);
        }
    }
    */

    SubmissionTask::SubmissionTask(LogicalDevice& logicalDevice, Queue& queue, std::string_view name)
        : logicalDevice{logicalDevice}, queue{queue},
        cmdPool{logicalDevice, queue, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT},
        cmdBuffers{ cmdPool.AllocateCommandsPerFlight(VK_COMMAND_BUFFER_LEVEL_PRIMARY) },
        //signal{signals},
        //full_workload{nullptr},
        //external_workload{nullptr},
        packaged_tasks{},
        name{name}
    {
#if EWE_DEBUG_NAMING
        for (uint8_t frame = 0; frame < EWE::max_frames_in_flight; frame++) {
            std::string debug_name = std::string(name) + "submission task [" + std::to_string(frame) + ']';
            cmdBuffers[frame].SetDebugName(debug_name);

            submitInfo[frame].AddCommandBuffer(cmdBuffers[frame]);
        }
#endif
    }

    bool SubmissionTask::Execute(uint8_t frameIndex) {

        bool ret = false;
        //if(full_workload){

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

            for(auto& task_package : packaged_tasks){
                ret |= task_package(cmdBuffers[frameIndex], frameIndex);
            }
#if EWE_DEBUG_NAMING
            logicalDevice.EndLabel(cmdBuffers[frameIndex]);
#endif

            cmdBuffers[frameIndex].End();

            cmdBuffers[frameIndex].state = CommandBuffer::State::Pending;
        //}
        //else if(external_workload){
        //    ret = external_workload(submitInfo[frameIndex], frameIndex);
        //}
        //else{
         //   EWE_ASSERT(false && "no active workload"); 
        //}

        return ret;
    }


    SubmissionBridge::SubmissionBridge(std::span<SubmissionTask*> lhs, SubmissionTask* rhs)
        : lhs{lhs}, rhs{rhs}
    {
        for (uint8_t i = 0; i < max_frames_in_flight; i++) {
            for (auto& lh : lhs) {
                //EWE_ASSERT(lh->signal); //i dont know if i want to force this or not
                rhs->submitInfo[i].WaitOnPrevious(lh->submitInfo[i]);
            }
        }
        
    }
    
}