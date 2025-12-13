#pragma once

//https://github.com/Ak-Elements/Onyx/blob/main/onyx/modules/graphics/public/onyx/graphics/rendergraph/rendergraph.h
//https://themaister.net/blog/2017/08/15/render-graphs-and-vulkan-a-deep-dive/

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/LogicalDevice.h"
#include "EightWinds/Swapchain.h"
#include "EightWinds/Queue.h"

#include "EightWinds/RenderGraph/GPUTask.h"
#include "EightWinds/RenderGraph/TaskBridge.h"
#include "EightWinds/RenderGraph/PresentBridge.h"
#include "EightWinds/RenderGraph/SubmissionTask.h"

#include "EightWinds/Data/Hive.h"

#include <span>

namespace EWE{
    struct RenderGraph{
        [[nodiscard]] explicit RenderGraph(LogicalDevice& logicalDevice, Swapchain& swapchain); //this cant be noexcept because Hive can throw if malloc fails
        RenderGraph(RenderGraph const& copySrc) = delete;
        RenderGraph(RenderGraph&& moveSrc) = delete;
        RenderGraph& operator=(RenderGraph const& copySrc) = delete;
        RenderGraph& operator=(RenderGraph&& moveSrc) = delete;

        bool Acquire(uint8_t frameIndex);

        LogicalDevice& logicalDevice;
        Swapchain& swapchain;

        //its important that task dont get moved or copied
        Hive<GPUTask> tasks;
        
        //links in the visual graph. if i change the task references to pointers, i can use this directly in a rendergraph without an intermediary
        //moving doesnt matter much, i just want stable references
        //Hive<TaskBridge> bridges;

        Hive<SubmissionTask> submissions;

        PresentBridge presentBridge;
        std::vector<std::vector<SubmissionTask*>> execution_order;


        VkResult presentResult = VK_SUCCESS;
        VkPresentInfoKHR presentInfo{};

        void Execute(CommandBuffer& cmdBuf, uint8_t frameIndex);
        void PresentBridge(CommandBuffer& cmdBuf);
        void Present();
    };
}//namespace EWE