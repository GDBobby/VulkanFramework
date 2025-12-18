#include "EightWinds/RenderGraph/PresentSubmission.h"

namespace EWE{
    PresentSubmission::PresentSubmission(Swapchain& swapchain, Queue& presentQueue)
        : swapchain{ swapchain },
        presentQueue{ presentQueue },
        incomingSemaphores{},
        presentInfo{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .swapchainCount = 1,
            .pSwapchains = &swapchain.activeSwapchain,
            .pImageIndices = &swapchain.imageIndex,
            .pResults = &presentResult
        }
    {}

    void PresentSubmission::WaitOnPrevious(std::span<SubmissionTask*> previous_group) {
        for (uint8_t i = 0; i < max_frames_in_flight; i++) {
            incomingSemaphores[i].clear();
            for (auto& previous : previous_group) {
                for (auto& semInfo : previous->submitInfo[i].signalSemaphores) {
                    incomingSemaphores[i].push_back(semInfo.semaphore);
                }
            }
        }
    }

    void PresentSubmission::WaitOnPrevious(SubmissionTask const& previous) {
        for (uint8_t i = 0; i < max_frames_in_flight; i++) {
            incomingSemaphores[i].clear();
            for (auto& semInfo : previous.submitInfo[i].signalSemaphores) {
                incomingSemaphores[i].push_back(semInfo.semaphore);
            }
        }
    }

    void PresentSubmission::Present(uint8_t frameIndex) {
        auto& frame_sems = incomingSemaphores[frameIndex];
        presentInfo.waitSemaphoreCount = static_cast<uint32_t>(frame_sems.size());
        presentInfo.pWaitSemaphores = frame_sems.data();
        EWE_VK(vkQueuePresentKHR, presentQueue, &presentInfo);
        EWE_VK_RESULT(presentResult);
    }
}//namespace EWE