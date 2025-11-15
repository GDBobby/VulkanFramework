#include "EightWinds/Backend/Semaphore.h"

#include <cassert>

namespace EWE{
    Semaphore::Semaphore(LogicalDevice& logicalDevice, bool timelineSemaphore, uint8_t initialValue)
        :logicalDevice{logicalDevice}
    {
        if(timelineSemaphore){
            //VK_SEMAPHORE_TYPE_BINARY = 0,
            //VK_SEMAPHORE_TYPE_TIMELINE = 1,

            //i need to find a way to rectify features internally
            printf("not checking features to see if timeline semaphore is enabled yet\n");
            //assert(logicalDevice.physicalDevice.features.timelineSemaphore);

            VkSemaphoreTypeCreateInfo timelineCreateInfo;
            timelineCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
            timelineCreateInfo.pNext = NULL;
            timelineCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
            timelineCreateInfo.initialValue = initialValue;

            VkSemaphoreCreateInfo semaphoreCreateInfo;
            semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            semaphoreCreateInfo.pNext = &timelineCreateInfo;
            semaphoreCreateInfo.flags = 0;

            EWE_VK(vkCreateSemaphore, logicalDevice.device, &semaphoreCreateInfo, nullptr, &vkSemaphore);
        }
        else{
            VkSemaphoreCreateInfo semaphoreCreateInfo;
            semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            semaphoreCreateInfo.pNext = nullptr;
            semaphoreCreateInfo.flags = 0;
            EWE_VK(vkCreateSemaphore, logicalDevice.device, &semaphoreCreateInfo, nullptr, &vkSemaphore);
        }
    }
}