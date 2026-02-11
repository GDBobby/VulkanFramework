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
        [[nodiscard]] explicit Semaphore(LogicalDevice& logicalDevice);
        Semaphore(Semaphore const& copySrc) = delete;
        Semaphore& operator=(Semaphore const& copySrc) = delete;
        Semaphore(Semaphore&& moveSrc) noexcept;
        Semaphore& operator=(Semaphore&& moveSrc) noexcept;
        ~Semaphore();
        
#if EWE_DEBUG_NAMING
        std::string debugName;
        void SetName(std::string_view name);
#endif

        operator VkSemaphore() const {
            return vkSemaphore;
        }
    };
    
    struct TimelineSemaphore{
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
        [[nodiscard]] explicit TimelineSemaphore(LogicalDevice& logicalDevice, uint64_t initialValue = 0);
        TimelineSemaphore(TimelineSemaphore const& copySrc) = delete;
        TimelineSemaphore& operator=(TimelineSemaphore const& copySrc) = delete;
        TimelineSemaphore(TimelineSemaphore&& moveSrc) noexcept;
        TimelineSemaphore& operator=(TimelineSemaphore&& moveSrc) noexcept;
        ~TimelineSemaphore();
        
#if EWE_DEBUG_NAMING
        std::string debugName;
        void SetName(std::string_view name);
#endif

        operator VkSemaphore() const {
            return vkSemaphore;
        }
    };
}