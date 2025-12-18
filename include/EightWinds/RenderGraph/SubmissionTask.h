#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/Queue.h"
#include "EightWinds/Command/CommandPool.h"
#include "EightWinds/Command/CommandBuffer.h"

#include "EightWinds/PerFlight.h"

#include "EightWinds/RenderGraph/GPUTask.h"
#include "EightWinds/RenderGraph/TaskBridge.h"
#include "EightWinds/Backend/Semaphore.h"

#include "EightWinds/Backend/SubmitInfo.h"

#include <optional>

namespace EWE{
    struct TaskSubmissionWorkload {

        //i dont know how to handle the bridges yet
        //for the moment im gonna automatically generate them
        std::vector<GPUTask*> ordered_gpuTasks;
        std::vector<TaskBridge> bridges;

        void GenerateBridges();

        bool Execute(CommandBuffer& cmdBuf);

        constexpr auto PackIntoTask() {
            return [this](CommandBuffer& cmdBuf) {
                return Execute(cmdBuf);
            };
        }
    };

    struct SubmissionTask{
        LogicalDevice& logicalDevice;
        Queue& queue;
        CommandPool cmdPool;
        PerFlight<CommandBuffer> cmdBuffers;
        bool signal;
        std::string name;
        [[nodiscard]] explicit SubmissionTask(LogicalDevice& logicalDevice, Queue& queue, bool signals, std::string_view name);

        PerFlight<Backend::SubmitInfo> submitInfo;

        PerFlight<Semaphore> signal_semaphores;

        //im abstracting the workload, so that the programmer isn't forced into using GPUTask.
        //the particular situation i have in mind is imgui, in which there is already a substantial existing workload
        std::function<bool(CommandBuffer&)> full_workload;
        //do i need to give the command buffer at all? or can i make it always explicit
        //for the moment i think i just take the L on the branch
        //this forces the user to define submitInfo before the render loop is initiated, or per frame
        //if this is used, an extra wasted command pool will exist. probably not a big deal?
        std::function<bool(Backend::SubmitInfo&, uint8_t frameIndex)> external_workload;

        //i dont really want to automatically generate barriers but we will for the moment
        bool Execute(uint8_t frameIndex);
    };

    //i dont think this object is necessary. idk
    struct SubmissionBridge{ 
        //this wont care about access, only queue and layout
        std::span<SubmissionTask*> lhs;
        SubmissionTask* rhs;

        [[nodiscard]] explicit SubmissionBridge(std::span<SubmissionTask*> lhs, SubmissionTask* rhs);
    };
}