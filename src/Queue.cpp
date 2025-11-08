#include "EightWinds/Queue.h"

namespace EWE {
    Queue::Queue(VkDevice logicalDeviceExplicit, QueueFamily& family, float priority)
        : family{family}, priority{priority}
    {
        vkGetDeviceQueue(logicalDeviceExplicit, family.index, 0, &queue);
    }

}