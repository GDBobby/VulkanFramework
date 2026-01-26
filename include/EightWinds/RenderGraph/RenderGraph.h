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

#include "EightWinds/Data/Hive.h"

#include <span>

namespace EWE{


    struct SynchronizationManager {
        template<typename Resource>
        struct TransitionObjects {
            GPUTask* lhs;
            uint32_t lh_index;
            GPUTask* rhs;
            uint32_t rh_index;
        };
        template<typename Resource>
        struct AcquireObjects{
            GPUTask* rhs;
            uint32_t rh_index;
        };

        std::vector<TransitionObjects<Buffer>> buffer_transitions;
        std::vector<TransitionObjects<Image>> image_transitions;
        std::vector<AcquireObjects<Buffer>> buffer_acquisitions;
        std::vector<AcquireObjects<Image>> image_acquisitions;

        void AddTransition_Buffer(GPUTask& lhs, uint32_t lh_index, GPUTask& rhs, uint32_t rh_index);
        void AddTransition_Image(GPUTask& lhs, uint32_t lh_index, GPUTask& rhs, uint32_t rh_index);
        void AddAcquisition_Buffer(GPUTask& rhs, uint32_t rh_index);
        void AddAcquisition_Image(GPUTask& rhs, uint32_t rh_index);

        void PopulateTasks();

    };


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
    };
}//namespace EWE