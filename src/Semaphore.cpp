#include "EightWinds/Semaphore.h"

namespace EWE{
    Semaphore::Semaphore(LogicalDevice& logicalDevice, bool timelineSemaphore, uint8_t initialValue)
        :logicalDevice{logicalDevice}
    {
        if(timelineSemaphore){
            //VK_SEMAPHORE_TYPE_BINARY = 0,
            //VK_SEMAPHORE_TYPE_TIMELINE = 1,

            assert(logicalDevice.physicalDevice.features.timelineSemaphore);

            VkSemaphoreTypeCreateInfo timelineCreateInfo;
            timelineCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
            timelineCreateInfo.pNext = NULL;
            timelineCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
            timelineCreateInfo.initialValue = initialValue;

            VkSemaphoreCreateInfo semaphoreCreateInfo;
            createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            createInfo.pNext = &timelineCreateInfo;
            createInfo.flags = 0;

            EWE_VK(vkCreateSemaphore, logicalDevice.device, &createInfo, NULL, &vkSemaphore);
        }
        else{
            VkSemaphoreCreateInfo semaphoreCreateInfo;
            createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            createInfo.pNext = nullptr;
            createInfo.flags = 0;

            EWE_VK(vkCreateSemaphore, logicalDevice.device, &createInfo, NULL, &vkSemaphore);
        }
    }
}