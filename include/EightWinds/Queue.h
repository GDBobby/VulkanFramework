#pragma once
#include "EightWinds/VulkanHeader.h"
#include "EightWinds/Backend/QueueFamily.h"

#include <mutex>

namespace EWE{
    struct LogicalDevice;
    struct Queue {
        LogicalDevice& logicalDevice;
        //i believe its safe to assume that Graphics and Present can always be the same queue
        //nvidia will allow multiple queues from 1 family, otherwise i wouldn't differentiate this from QueueFamily
        QueueFamily const& family;

        float priority;
        VkQueue queue;

        std::mutex mut;

        [[nodiscard]] explicit Queue(LogicalDevice& logicalDevice, QueueFamily const& family, float priority);
        Queue(Queue const& copySrc) = delete;
        Queue& operator=(Queue const& copySrc) = delete;
        //just so the logicalDevice vector quits bitching at me. 
        //i dont feel like making a heap array class. 
        //do not move this.
        Queue(Queue&& moveSrc) noexcept;
        Queue& operator=(Queue&& moveSrc) = delete;

        void Submit(uint32_t submitCount, VkSubmitInfo* submitInfos, VkFence fence) const;
        void Submit2(uint32_t submitCount, VkSubmitInfo2* submitInfos, VkFence fence) const;
        
        uint32_t FamilyIndex() const{
            return family.index;
        }

        //TODO
		//void BeginLabel(const char* name, float red, float green, float blue);
		//void EndLabel();

        operator VkQueue() const { return queue; }

#if EWE_DEBUG_NAMING
        std::string debugName;
        void SetName(std::string_view name);
#endif
    };
}