#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/RenderGraph/SubmissionTask.h"
#include "EightWinds/Swapchain.h"

namespace EWE{
    struct PresentSubmission {
        Swapchain& swapchain;
        Queue& presentQueue;
        PerFlight<std::vector<VkSemaphore>> incomingSemaphores;
        VkPresentInfoKHR presentInfo;
        VkResult presentResult;

        [[nodiscard]] explicit PresentSubmission(Swapchain& swapchain, Queue& presentQueue);

        void WaitOnPrevious(std::span<SubmissionTask*> previous_group);

        void WaitOnPrevious(SubmissionTask const& previous);

        void Present(uint8_t frameIndex);
    };
}//namespace EWE