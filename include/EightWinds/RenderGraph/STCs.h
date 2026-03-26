#pragma once

#include "EightWinds/Queue.h"
#include "EightWinds/RenderGraph/SubmissionTask.h"
#include "EightWinds/Backend/Semaphore.h"
#include "EightWinds/RenderGraph/Resources.h"

#include "EightWinds/Image.h"
#include "EightWinds/Buffer.h"

#include "EightWinds/Data/RingBuffer.h"

#include <vector>


namespace EWE{

    struct STCManagement{
        LogicalDevice& logicalDevice;

        Queue& graphicsQueue;
        Queue& computeQueue;
        SubmissionTask& graphics_stc_task;
        SubmissionTask& compute_stc_task;

        [[nodiscard]] STCManagement(LogicalDevice& logicalDevice, 
            Queue& graphicsQueue, Queue& computeQueue, 
            SubmissionTask& graphics_stc_task, SubmissionTask& compute_stc_task
        );

        template<typename R>
        struct Helper{
            R::Barrier barrier;
            Resource<R> res;
            Queue* dstQueue;
        };
        std::vector<Helper<Image>> image_ownership;
        std::vector<Helper<Buffer>> buffer_ownership;

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