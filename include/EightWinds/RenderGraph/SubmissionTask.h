#pragma once

#include "EightWinds/Queue.h"
#include "EightWinds/CommandPool.h"
#include "EightWinds/CommandBuffer.h"

#include "EightWinds/Data/PerFlight.h"

#include "EightWinds/Backend/Semaphore.h"

#include "EightWinds/Backend/SubmitInfo.h"
#include "GPUTask.h"

namespace EWE{

    struct SubmissionTask{
        LogicalDevice& logicalDevice;
        Queue* queue; //not optional
        CommandPool cmdPool;
        PerFlight<CommandBuffer> cmdBuffers;
        //bool signal;
        std::string name;
        PerFlight<Backend::SubmitInfo> submitInfo;

        //specialized tasks are like the imgui task, which doesnt use GPUTask
        //more specifically, they aren't generated in graphs, but 'hand' coded
        bool specializedSubmission = false; 

        std::vector<std::function<bool(CommandBuffer& cmdBuf, uint8_t frameIndex)>> packaged_tasks;
        std::vector<GPUTask*> tasks; //this isn't really necessary during runtime, just for storing the data necessary for reconstruction

        bool uses_present_image = false;
        
        [[nodiscard]] explicit SubmissionTask(std::string_view name, LogicalDevice& logicalDevice, Queue& queue);

        void CollectTaskWorkloads();
        bool Execute(uint8_t frameIndex);
    };

    //this is a helper struct that will build the semaphore wait data for each submission task
    struct SubmissionBridge{ 
        //this wont care about access, only queue and layout
        std::span<SubmissionTask*> lhs;
        SubmissionTask* rhs;

        [[nodiscard]] explicit SubmissionBridge(std::span<SubmissionTask*> lhs, SubmissionTask* rhs);
    };
}