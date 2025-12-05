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
    Semaphore::~Semaphore() {
        if (vkSemaphore != VK_NULL_HANDLE) {
            vkDestroySemaphore(logicalDevice.device, vkSemaphore, nullptr);
        }
    }
    
    Semaphore::Semaphore(Semaphore&& moveSrc) noexcept
        : logicalDevice{moveSrc.logicalDevice},
        vkSemaphore{moveSrc.vkSemaphore},
        waiting{moveSrc.waiting},
        signaling{moveSrc.signaling}
    {
        moveSrc.vkSemaphore = VK_NULL_HANDLE;
    }
    Semaphore& Semaphore::operator=(Semaphore&& moveSrc) noexcept{
        assert(logicalDevice == moveSrc.logicalDevice);
        vkSemaphore = moveSrc.vkSemaphore;
        moveSrc.vkSemaphore = VK_NULL_HANDLE;

        waiting = moveSrc.waiting;
        signaling = moveSrc.signaling;
        return *this;
    }
    
#if EWE_DEBUG_NAMING
        void Semaphore::SetName(std::string_view name){
            logicalDevice.SetObjectName(vkSemaphore, VK_OBJECT_TYPE_SEMAPHORE, name);
        }
#endif
}