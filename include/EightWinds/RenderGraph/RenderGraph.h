#pragma once

//https://github.com/Ak-Elements/Onyx/blob/main/onyx/modules/graphics/public/onyx/graphics/rendergraph/rendergraph.h
//https://themaister.net/blog/2017/08/15/render-graphs-and-vulkan-a-deep-dive/

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/LogicalDevice.h"
#include "EightWinds/Swapchain.h"
#include "EightWinds/Queue.h"

#include "EightWinds/RenderGraph/GPUTask.h"
#include "EightWinds/RenderGraph/TaskBridge.h"

#include "EightWinds/Data/Hive.h"

#include <variant>

namespace EWE{
    struct RenderGraph{
        [[nodiscard]] explicit RenderGraph(LogicalDevice& logicalDevice, Swapchain& swapchain, Queue& presentQueue); //this cant be noexcept because Hive can throw if malloc fails
        RenderGraph(RenderGraph const& copySrc) = delete;
        RenderGraph(RenderGraph&& moveSrc) = delete;
        RenderGraph& operator=(RenderGraph const& copySrc) = delete;
        RenderGraph& operator=(RenderGraph&& moveSrc) = delete;

        bool Acquire(uint8_t frameIndex);

        LogicalDevice& logicalDevice;
        Swapchain& swapchain;
        Queue& presentQueue;

        //i want a specialized PresentTask

        //its important that task dont get moved or copied
        Hive<GPUTask> tasks;
        
        //links in the visual graph. if i change the task references to pointers, i can use this directly in a rendergraph without an intermediary
        //moving doesnt matter much, i just want stable references
        Hive<TaskBridge> bridges;

        //i can automatically generated TaskBridges, i believe. the only issue, is I don't want to reallocate the memory every frame.
        //we could potentially be talking about 1000s of barriers, if it's 1 alloc per frame that's acceptable, but i think it could
        //be done with 0 allocs per frame without much trouble
        std::vector<std::variant<GPUTask*, TaskBridge*>> execution_order;


        VkResult presentResult = VK_SUCCESS;
        VkPresentInfoKHR presentInfo{};
        

        void Execute(CommandBuffer& cmdBuf, uint8_t frameIndex);

        void Present();
    };
}//namespace EWE