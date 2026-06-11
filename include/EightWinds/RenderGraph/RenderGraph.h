#pragma once

//https://github.com/Ak-Elements/Onyx/blob/main/onyx/modules/graphics/public/onyx/graphics/rendergraph/rendergraph.h
//https://themaister.net/blog/2017/08/15/render-graphs-and-vulkan-a-deep-dive/

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/LogicalDevice.h"
#include "EightWinds/Swapchain.h"
#include "EightWinds/Queue.h"

#include "EightWinds/RenderGraph/GPUTask.h"
//#include "EightWinds/RenderGraph/PresentSubmission.h"
#include "EightWinds/RenderGraph/SubmissionTask.h"
#include "EightWinds/RenderGraph/PresentBridge.h"
#include "EightWinds/RenderGraph/SynchronizationManager.h"

#include "EightWinds/Backend/Semaphore.h"
#include "EightWinds/Command/STC_Manager.h"

#include "EightWinds/Data/Hive.h"
#include "EightWinds/Data/RingBuffer.h"

namespace EWE{

    struct ImguiExtension;

    struct RenderGraph{
        std::filesystem::path name;

        [[nodiscard]] explicit RenderGraph(
            std::filesystem::path const& name,
            LogicalDevice& logicalDevice, Swapchain& swapchain, 
            Queue& renderQueue, Queue& computeQueue,
            SubmissionTask& graphics_stc_task,
            SubmissionTask& compute_stc_task
        );
        RenderGraph(RenderGraph const& copySrc) = delete;
        RenderGraph(RenderGraph&& moveSrc) = delete;
        RenderGraph& operator=(RenderGraph const& copySrc) = delete;
        RenderGraph& operator=(RenderGraph&& moveSrc) = delete;

        bool Acquire(uint8_t frameIndex);

        LogicalDevice& logicalDevice;
        Swapchain& swapchain;
        Queue& renderQueue;
        Queue& computeQueue;

        SubmissionTask& graphics_stc_task;
        SubmissionTask& compute_stc_task;

        PresentBridge presentBridge;
        SynchronizationManager syncManager;

        std::vector<GPUTask*> tasks; //this is for adjusting prefixes
        //each inner vector needs to use only one queue
        std::vector<std::vector<SubmissionTask*>> execution_order;

        PerFlight<VkSemaphoreSubmitInfo> present_wait_semaphore_data;
        VkResult presentResult = VK_SUCCESS;
        PerFlight<VkPresentInfoKHR> presentInfo;
        PerFlight<std::vector<VkSemaphore>> present_wait_raw_semaphore_data{};
        //PerFlight<std::vector<VkSemaphore*>> present_acquire_waits{};

        PerFlight<RuntimeArray<TimelineSemaphore>> semaphores;
        int first_graphics_task_group = -1; //under 0 means it's invalid
        int first_compute_task_group = -1;

        void Execute(uint8_t frameIndex);

        struct SwapImageInstance{
            GPUTask* task;
            uint32_t res_index;
        };
        std::vector<SwapImageInstance> swap_image_instances;
        void UpdateSwapImage(uint8_t frameIndex);
        void ClearAllBarriers(uint8_t frameIndex);
        void RecreateBarriers(uint8_t frameIndex);

        void InitializeSemaphores(); //this prevents needing to branch every frame
        //this needs to be ran every frame
        void UpdateSemaphores(uint8_t frameIndex, STC_Submitter& frame_stc_manager);

        RingBuffer<STC_Submitter, max_frames_in_flight + 1> stc_management;
        STC_Submitter* current_stc_manager;

        template<ResourceType R>
        void ResourceOwnershipTransfer(STC_Sub_Package<R> const& data);

        template<ResourceType T>
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

    template <> void RenderGraph::ResourceOwnershipTransfer(STC_Sub_Package<Image> const& data);
    template <> void RenderGraph::ResourceOwnershipTransfer(STC_Sub_Package<Buffer> const& data);
}//namespace EWE