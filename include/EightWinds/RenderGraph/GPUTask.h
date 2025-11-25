#pragma once

#include "EightWinds/VulkanHeader.h"

#include "EightWinds/Backend/RenderInfo.h"
#include "EightWinds/Command/Execute.h"

#include <span>

//equivalent a renderpass subpass?

namespace EWE{

    //the id of this task is its address
    struct GPUTask{
        GPUTask* feedsInto = nullptr; //if its nullptr its a present/submission

        CommandExecutor commandExecutor{};

        //idk if i want to commit to renderInfo here yet
        //std::optional<RenderInfo> renderInfo;

        GPUTask() = default;
        GPUTask(GPUTask const&) = delete;
        GPUTask& operator=(GPUTask const&) = delete;

        //i dont know if i want it to be movable or not yet, but for now this is fine
        GPUTask(GPUTask&&) = default;
        GPUTask& operator=(GPUTask&&) = delete;

        //im not committed to putting the command buffer here. 
        //i might let each GPUTask create its own command buffer on execution
        //void Execute(CommandBuffer& cmdBuf) const noexcept;
    };
}