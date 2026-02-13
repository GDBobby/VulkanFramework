#pragma once

//https://github.com/Ak-Elements/Onyx/blob/main/onyx/modules/graphics/public/onyx/graphics/rendergraph/rendergraph.h
//https://themaister.net/blog/2017/08/15/render-graphs-and-vulkan-a-deep-dive/

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/LogicalDevice.h"
#include "EightWinds/Swapchain.h"
#include "EightWinds/Queue.h"

#include "EightWinds/RenderGraph/GPUTask.h"
#include "EightWinds/RenderGraph/PresentSubmission.h"
#include "EightWinds/RenderGraph/SubmissionTask.h"
#include "EightWinds/RenderGraph/PresentBridge.h"
#include "EightWinds/RenderGraph/SynchronizationManager.h"

#include "EightWinds/Data/Hive.h"

#include <span>

namespace EWE{

    struct ImguiExtension;



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
        
        Hive<SubmissionTask> submissions;
        PresentSubmission presentSubmission;

        PresentBridge presentBridge;

        SynchronizationManager syncManager;

        //each inner vector needs to use only one queue
        std::vector<std::vector<SubmissionTask*>> execution_order;

        VkResult presentResult = VK_SUCCESS;
        VkPresentInfoKHR presentInfo{};

        void Execute(uint8_t frameIndex);

        void ClearAllBarriers(uint8_t frameIndex);
        void RecreateBarriers(uint8_t frameIndex);

        template<typename T>
        void ChangeResource(GPUTask& task, uint32_t res_index, T* resource, uint8_t frameIndex) {
            if constexpr (std::is_same_v<T, Image>) {
                task.resources.images[res_index].resource[frameIndex] = resource;
            }
            else if constexpr (std::is_same_v<T, Buffer>) {
                task.resources.buffers[res_index].resource[frameIndex] = resource;
            }
        }


        friend struct ImguiExtension;
    };
}//namespace EWE