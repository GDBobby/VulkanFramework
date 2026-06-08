#pragma once


//im getting stunlocked, i need to put this off

#include "EightWinds/VulkanHeader.h"

#include <functional>
#include <mutex>

namespace EWE{

    struct LogicalDevice;

namespace Backend{

    struct GarbageItem{
        uint8_t tossedDuration = 0;

        std::function<void()> Destroy = nullptr;

        [[nodiscard]] GarbageItem() = default;
        GarbageItem& operator=(GarbageItem const& copySrc) = delete;
        GarbageItem& operator=(GarbageItem&& moveSrc);
        GarbageItem(GarbageItem const& copySrc) = delete;
        [[nodiscard]] GarbageItem(GarbageItem&& moveSrc);

        ~GarbageItem(){
            if(Destroy != nullptr){
                Destroy();
            }
        }
    };

    struct GarbageDisposal{
        LogicalDevice& device;

        std::vector<GarbageItem> items{};
        std::mutex item_mut{};

        [[nodiscard]] explicit GarbageDisposal(LogicalDevice& device);
        ~GarbageDisposal();

        void Clear();
        void Tick();

        void Toss(std::function<void()> destroyer);
        template<typename T>
        void TossVK(T func);
    };

    template<> void GarbageDisposal::TossVK(VkSemaphore sem);
    template<> void GarbageDisposal::TossVK(VkFence fence);
    template<> void GarbageDisposal::TossVK(VkBuffer buffer);
    template<> void GarbageDisposal::TossVK(VkSampler sampler);
} //namespace backend
} //namespace EWE