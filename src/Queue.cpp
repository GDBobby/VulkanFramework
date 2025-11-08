#include "EightWinds/Queue.h"

namespace EWE {
    Queue::Queue(vkDevice logicalDeviceExplicit, QueueFamily& family, float priority)
        : family{family}, priority{priority}
    {
        logicalDeviceExplicit.getQueue(family.index, 0, &queue);
    }

}