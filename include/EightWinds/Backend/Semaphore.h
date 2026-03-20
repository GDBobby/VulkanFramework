#pragma once
#include "EightWinds/VulkanHeader.h"
#include "EightWinds/LogicalDevice.h"
#include <vulkan/vulkan_core.h>

namespace EWE{
    struct BinarySemaphore {
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
        [[nodiscard]] explicit BinarySemaphore(LogicalDevice& logicalDevice);
        BinarySemaphore(BinarySemaphore const& copySrc) = delete;
        BinarySemaphore& operator=(BinarySemaphore const& copySrc) = delete;
        BinarySemaphore(BinarySemaphore&& moveSrc) noexcept;
        BinarySemaphore& operator=(BinarySemaphore&& moveSrc) noexcept;
        ~BinarySemaphore();
        
#if EWE_DEBUG_NAMING
        std::string debugName;
        void SetName(std::string_view name);
#endif

        bool operator==(BinarySemaphore const& other) const{
            return vkSemaphore == other.vkSemaphore;
        }
        operator VkSemaphore() const {
            return vkSemaphore;
        }
    };
    
    struct TimelineSemaphore{
        LogicalDevice& logicalDevice;
        VkSemaphore vkSemaphore{ VK_NULL_HANDLE };

        uint64_t value;
        
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
        [[nodiscard]] TimelineSemaphore(TimelineSemaphore&& moveSrc) noexcept;
        TimelineSemaphore& operator=(TimelineSemaphore&& moveSrc) noexcept;
        ~TimelineSemaphore();

        VkSemaphoreSubmitInfo GetSignalSubmitInfo(VkPipelineStageFlags2 stageMask) noexcept;
        VkSemaphoreSubmitInfo GetWaitSubmitInfo(VkPipelineStageFlags2 stageMask) const noexcept;
        VkSemaphoreSubmitInfo GetSubmitInfo(VkPipelineStageFlags2 stageMask, bool signal) noexcept;
        void WaitOn(uint64_t val);
        uint64_t GetCurrentValue() const;
        bool Check(uint64_t val) const;
        
#if EWE_DEBUG_NAMING
        std::string debugName;
        void SetName(std::string_view name);
#endif

        bool operator==(TimelineSemaphore const& other) const{
            return vkSemaphore == other.vkSemaphore;
        }
        operator VkSemaphore() const {
            return vkSemaphore;
        }
    };
}