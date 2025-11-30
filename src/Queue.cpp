#include "EightWinds/Queue.h"

namespace EWE {
    Queue::Queue(VkDevice logicalDeviceExplicit, QueueFamily const& family, float priority)
        : family{family}, priority{priority}
    {
        vkGetDeviceQueue(logicalDeviceExplicit, family.index, 0, &queue);
    }

    void Queue::Submit(uint32_t submitCount, VkSubmitInfo* submitInfos, VkFence fence) const{
        EWE_VK(vkQueueSubmit, queue, submitCount, submitInfos, fence);
    }
}