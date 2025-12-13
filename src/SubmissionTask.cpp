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

    SubmissionTask::SubmissionTask(LogicalDevice& logicalDevice, Queue& queue, bool signals)
        : queue{queue},
        cmdPool{logicalDevice, queue, VK_COMMAND_POOL_CREATE_FLAGS},
        signal{signals},
        signal_semaphores{logicalDevice, false, 0},
        implicit_workload{nullptr},
        explicit_workload{nullptr}
    {

    }

    void SubmissionTask::Execute() {

        if(implicit_workload){
            CommandBuffer cmdBuf{cmdPool, VK_NULL_HANDLE};

            //beginInfo
            cmdBuf.Begin();

            implicit_workload(cmdBuf);

            cmdBuf.End();
        }
        else if(explicit_workload){
            explicit_workload();
        }
        else{
            assert(false && "no active workload"); 
        }

        auto subInfo = submitInfo.Expand();

        queue.Submit2(1, &subInfo, VK_NULL_HANDLE);
    }
}