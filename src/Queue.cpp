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

    //Queue::Queue(Queue&& moveSrc) noexcept
    //: logicalDevice{moveSrc.logicalDevice},
    //    family{moveSrc.family},
    //    priority{moveSrc.priority},
    //    queue{moveSrc.queue}
    //{
    //    //POTENTIALLY put a debug only stacktrace assert here, that asserts the move source was from logicaldevice internals
    //}

    void Queue::Submit(uint32_t submitCount, VkSubmitInfo* submitInfos, VkFence fence) {
        std::unique_lock subLock{mut};
        EWE_VK(vkQueueSubmit, queue, submitCount, submitInfos, fence);
    }
    void Queue::Submit2(VkSubmitInfo2 const& submitInfo, VkFence fence) {
        std::unique_lock subLock{mut};
        EWE_VK(vkQueueSubmit2, queue, 1, &submitInfo, fence);
    }
    void Queue::Submit2(uint32_t submitCount, VkSubmitInfo2* submitInfos, VkFence fence) {
        std::unique_lock subLock{mut};
        EWE_VK(vkQueueSubmit2, queue, submitCount, submitInfos, fence);
    }


#if EWE_DEBUG_NAMING
    void Queue::SetName(std::string_view name) {
        logicalDevice.SetObjectName(queue, VK_OBJECT_TYPE_QUEUE, name);
    }
#endif
}