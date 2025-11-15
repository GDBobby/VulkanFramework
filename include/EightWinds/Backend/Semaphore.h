#pragma once
#include "EightWinds/VulkanHeader.h"
#include "EightWinds/LogicalDevice.h"

namespace EWE{
    struct Semaphore {
        LogicalDevice& logicalDevice;

        VkSemaphore vkSemaphore{ VK_NULL_HANDLE };
#if SEMAPHORE_TRACKING
        struct Tracking{
            enum State {
                BeginSignaling,
                FinishSignaling,
                BeginWaiting,
                FinishWaiting,
            };
            //this needs to be fixed with std::stacktrace
            State state;
            std::source_location srcLocation;
            Tracking(State state, std::source_location srcLocation) : state{ state }, srcLocation{ srcLocation } {}
        };
        std::vector<Tracking> tracking{};
#endif
        [[nodiscard]] explicit Semaphore(LogicalDevice& logicalDevice, bool timelineSemaphore, uint8_t initialValue = 0);

        bool waiting{ false };
        bool signaling{ false };

        bool Idle() const {
            return !(waiting || signaling);
        }
        void BeginSignaling();
        void FinishWaiting();
        void BeginWaiting();
    };
}