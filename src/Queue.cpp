#include "EightWinds/Queue.h"
#include "EightWinds/LogicalDevice.h"

namespace EWE {
    Queue::Queue(LogicalDevice& logicalDevice, QueueFamily const& family, float priority)
        : logicalDevice{ logicalDevice }, 
        family {family}, 
        priority{ priority }
    {
        vkGetDeviceQueue(logicalDevice.device, family.index, 0, &queue);
    }

    Queue::Queue(Queue&& moveSrc) noexcept
    : logicalDevice{moveSrc.logicalDevice},
        family{moveSrc.family},
        priority{moveSrc.priority},
        queue{moveSrc.queue}
    {
        //POTENTIALLY put a debug only stacktrace assert here, that asserts the move source was from logicaldevice internals
    }

    void Queue::Submit(uint32_t submitCount, VkSubmitInfo* submitInfos, VkFence fence) const{
        EWE_VK(vkQueueSubmit, queue, submitCount, submitInfos, fence);
    }
    void Queue::Submit2(uint32_t submitCount, VkSubmitInfo2* submitInfos, VkFence fence) const{
        EWE_VK(vkQueueSubmit2, queue, submitCount, submitInfos, fence);
    }
}