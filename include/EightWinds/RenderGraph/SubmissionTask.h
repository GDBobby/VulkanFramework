#pragma once

#include "EightWinds/Queue.h"
#include "EightWinds/CommandPool.h"
#include "EightWinds/CommandBuffer.h"

#include "EightWinds/Data/PerFlight.h"

#include "EightWinds/Backend/Semaphore.h"

#include "EightWinds/Backend/SubmitInfo.h"

namespace EWE{

    struct SubmissionTask{
        LogicalDevice& logicalDevice;
        Queue& queue;
        CommandPool cmdPool;
        PerFlight<CommandBuffer> cmdBuffers;
        //bool signal;
        std::string name;
        PerFlight<Backend::SubmitInfo> submitInfo;

        std::vector<std::function<bool(CommandBuffer& cmdBuf, uint8_t frameIndex)>> packaged_tasks;
        bool uses_present_image = false;
        
        [[nodiscard]] explicit SubmissionTask(LogicalDevice& logicalDevice, Queue& queue, std::string_view name);


        //i dont really want to automatically generate barriers but we will for the moment
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