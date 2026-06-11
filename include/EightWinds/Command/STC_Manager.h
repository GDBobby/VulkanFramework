#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/Backend/Semaphore.h"
#include "EightWinds/Backend/StagingBuffer.h"
#include "EightWinds/CommandPool.h"
#include "EightWinds/CommandBuffer.h"
#include "EightWinds/Command/STC.h"
#include "EightWinds/Command/STC_Helper.h"

#include "EightWinds/Data/RingBuffer.h"

#include <mutex>
#include <array>

namespace EWE{
    struct CommandBuffer;
    struct RenderGraph;

    class STC_Manager{
    public:
        RenderGraph* current_renderGraph = nullptr;
        Queue& renderQueue;
        Queue& transferQueue;
        Queue& computeQueue;

    private:
        RingBuffer<CommandPool, max_frames_in_flight * 2> renderCommandPools;
        //idk count, i want like 2 per fiber but i wont know fiber count at compile time
        
        std::array<std::mutex, Queue::COUNT> stc_mutexes;
        std::array<RingBuffer<CommandPool, 16>, Queue::COUNT> stc_command_pools;

        std::mutex semAcqMut{};
        RingBuffer<TimelineSemaphore, 32> semaphores;

        SingleTimeCommand* GetSTC(Queue::Type requested_queue);

        Queue::Type GetQueueType(Queue const& queue) const;
        Queue& GetQueueFromType(Queue::Type qType);

    public:
        [[nodiscard]] explicit STC_Manager(
            LogicalDevice& logicalDevice, 
            Queue& renderQueue, Queue& transferQueue, Queue& computeQueue
        );
        ~STC_Manager();

        STC_Manager(STC_Manager const& copySrc) = delete;
        STC_Manager(STC_Manager&& moveSrc) = delete;
        STC_Manager& operator=(STC_Manager const& copySrc) = delete;
        STC_Manager& operator=(STC_Manager&& moveSrc) = delete;

        TimelineSemaphore* GetSemaphore();
        void ReturnSemaphore(TimelineSemaphore* semaphore);

        bool CheckFencesForUsage();
        void CheckFencesForCallbacks();

        void AsyncTransferToCompute(std::function<void(CommandBuffer& cmdBuf)> transfer, std::function<void(CommandBuffer& cmdBuf)> compute);

        SingleTimeCommand* GetBeginSTC();
        TimelineSemaphore* EndAndSubmit(SingleTimeCommand& stc);

        template<ResourceType RT>
        void Async_SingleQueueTransfer(TransferContext<RT>& transferContext, Queue::Type dstQueueType);
        template<ResourceType RT>
        void AsyncTransfer_Helper(TransferContext<RT>& transferContext, Queue::Type dstQueueType);
        template<ResourceType RT>
        void AsyncTransfer(TransferContext<RT>& transferContext, Queue& rh_queue);
        template<ResourceType RT>
        void AsyncTransfer(TransferContext<RT>& transferContext, Queue::Type dstQueue);


        //this is done on the render queue exclusively
        template<ResourceType RT>
        void InlineTransfer(TransferContext<RT>& transferContext);
    };

    template<> void STC_Manager::Async_SingleQueueTransfer(TransferContext<Image>& transferContext, Queue::Type dstQueueType);
    template<> void STC_Manager::AsyncTransfer_Helper(TransferContext<Image>& transferContext, Queue::Type dstQueueType);
    //this will ONLY work in a separate thread
    template<> void STC_Manager::AsyncTransfer(TransferContext<Image>& transferContext, Queue& rh_queue);
    template<> void STC_Manager::AsyncTransfer(TransferContext<Image>& transferContext, Queue::Type dstQueue);

    template<> void STC_Manager::Async_SingleQueueTransfer(TransferContext<Buffer>& transferContext, Queue::Type dstQueueType);
    template<> void STC_Manager::AsyncTransfer_Helper(TransferContext<Buffer>& transferContext, Queue::Type dstQueueType);
    template<> void STC_Manager::AsyncTransfer(TransferContext<Buffer>& transferContext, Queue& rh_queue);
    template<> void STC_Manager::AsyncTransfer(TransferContext<Buffer>& transferContext, Queue::Type dstQueue);

    template<> void STC_Manager::InlineTransfer(TransferContext<Image>& transferContext);
    template<> void STC_Manager::InlineTransfer(TransferContext<Buffer>& transferContext);



    struct STC_Submitter{
        LogicalDevice& logicalDevice;

        //transfer queue isn't included because it's not a valid dst queue
        Queue& graphicsQueue;
        Queue& computeQueue;

        [[nodiscard]] STC_Submitter(
            LogicalDevice& logicalDevice, 
            Queue& graphicsQueue, Queue& computeQueue
        );

        std::vector<STC_Sub_Package<Image>> image_ownership;
        std::vector<STC_Sub_Package<Buffer>> buffer_ownership;

        bool CheckSize(Queue::Type qType) const;
        void CollectSTCs();
        void UpdateResources();

        std::function<bool(CommandBuffer& cmdBuf, uint8_t frameIndex)> graphics_task;
        std::function<bool(CommandBuffer& cmdBuf, uint8_t frameIndex)> compute_task;

        std::vector<Image::Barrier> graphics_image_barriers;
        std::vector<Buffer::Barrier> graphics_buffer_barriers;
        std::vector<Image::Barrier> compute_image_barriers;
        std::vector<Buffer::Barrier> compute_buffer_barriers;
        VkDependencyInfo graphicsInfo;
        VkDependencyInfo computeInfo;

        void Clear();
    };
} //namespace EWE