#pragma once
#include "EightWinds/VulkanHeader.h"
#include "EightWinds/QueueFamily.h"

namespace EWE{
    struct Queue {
        //i believe its safe to assume that Graphics and Present can always be the same queue
        //nvidia will allow multiple queues from 1 family, otherwise i wouldn't differentiate this from QueueFamily
        QueueFamily& family;

        float priority;
        VkQueue queue;

        //the queue itself doesnt really do any operations

        //this woudl mark the first time I break away from EWE wrapping structures. 
        //i feel like i either fully commit to them, or not at all
        //i either pass in the raw vkDevice for creation, or I pass in the vkQueue already created
        //which makes this struct somewhat pointless? it just ties a vkQueue to a family
        [[nodiscard]] explicit Queue(VkDevice logicalDeviceExplicit, QueueFamily& family, float priority);

        //TODO
		//void BeginLabel(const char* name, float red, float green, float blue);
		//void EndLabel();

        operator VkQueue() const { return queue; }
    };
}