#pragma once
#include "EightWinds/VulkanHeader.h"
#include "EightWinds/QueueFamily.h"

namespace EWE{
    //the queue itself doesnt really do any operations
    struct Queue {
        //i believe its safe to assume that Graphics and Present can always be the same queue
        //nvidia will allow multiple queues from 1 family, otherwise i wouldn't differentiate this from QueueFamily
        QueueFamily& family;

        float priority;
        VkQueue queue;

        [[nodiscard]] explicit Queue(VkDevice logicalDeviceExplicit, QueueFamily& family, float priority);

        //TODO
		//void BeginLabel(const char* name, float red, float green, float blue);
		//void EndLabel();

        operator VkQueue() const { return queue; }
    };
}