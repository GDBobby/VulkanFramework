#include "EightWinds/Backend/Semaphore.h"

#include <cassert>

namespace EWE{
    Semaphore::Semaphore(LogicalDevice& logicalDevice)
        :logicalDevice{logicalDevice}
    {
        VkSemaphoreCreateInfo semaphoreCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0
        };
        EWE_VK(vkCreateSemaphore, logicalDevice.device, &semaphoreCreateInfo, nullptr, &vkSemaphore);
    }
    Semaphore::~Semaphore() {
        if (vkSemaphore != VK_NULL_HANDLE) {
            logicalDevice.garbageDisposal.Toss(vkSemaphore, VK_OBJECT_TYPE_SEMAPHORE);
        }
    }
    
    Semaphore::Semaphore(Semaphore&& moveSrc) noexcept
        : logicalDevice{moveSrc.logicalDevice},
        vkSemaphore{moveSrc.vkSemaphore}//,
        //waiting{moveSrc.waiting},
        //signaling{moveSrc.signaling}
    {
        moveSrc.vkSemaphore = VK_NULL_HANDLE;
    }
    Semaphore& Semaphore::operator=(Semaphore&& moveSrc) noexcept{
        assert(logicalDevice == moveSrc.logicalDevice);
        vkSemaphore = moveSrc.vkSemaphore;
        moveSrc.vkSemaphore = VK_NULL_HANDLE;

        //waiting = moveSrc.waiting;
        //signaling = moveSrc.signaling;
        return *this;
    }
    
#if EWE_DEBUG_NAMING
        void Semaphore::SetName(std::string_view name){
            logicalDevice.SetObjectName(vkSemaphore, VK_OBJECT_TYPE_SEMAPHORE, name);
        }
#endif







    TimelineSemaphore::TimelineSemaphore(LogicalDevice& logicalDevice, uint64_t initialValue)
        : logicalDevice{logicalDevice}
    {
        //VK_SEMAPHORE_TYPE_BINARY = 0,
        //VK_SEMAPHORE_TYPE_TIMELINE = 1,

        //i need to find a way to rectify features internally
#if EWE_DEBUG_BOOL
        printf("not checking features to see if timeline semaphore is enabled yet\n");
        assert(logicalDevice.features12.timelineSemaphore);
#endif

        VkSemaphoreTypeCreateInfo timelineCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
            .pNext = NULL,
            .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
            .initialValue = initialValue
        };

        VkSemaphoreCreateInfo semaphoreCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = &timelineCreateInfo,
            .flags = 0
        };

        EWE_VK(vkCreateSemaphore, logicalDevice.device, &semaphoreCreateInfo, nullptr, &vkSemaphore);
    }
    TimelineSemaphore::~TimelineSemaphore() {
        if (vkSemaphore != VK_NULL_HANDLE) {
            logicalDevice.garbageDisposal.Toss(vkSemaphore, VK_OBJECT_TYPE_SEMAPHORE);
        }
    }
    
    TimelineSemaphore::TimelineSemaphore(TimelineSemaphore&& moveSrc) noexcept
        : logicalDevice{moveSrc.logicalDevice},
        vkSemaphore{moveSrc.vkSemaphore}//,
        //waiting{moveSrc.waiting},
        //signaling{moveSrc.signaling}
    {
        moveSrc.vkSemaphore = VK_NULL_HANDLE;
    }
    TimelineSemaphore& TimelineSemaphore::operator=(TimelineSemaphore&& moveSrc) noexcept{
#if EWE_DEBUG
        assert(logicalDevice == moveSrc.logicalDevice);
#endif
        vkSemaphore = moveSrc.vkSemaphore;
        moveSrc.vkSemaphore = VK_NULL_HANDLE;

        //waiting = moveSrc.waiting;
        //signaling = moveSrc.signaling;
        return *this;
    }
    
#if EWE_DEBUG_NAMING
        void TimelineSemaphore::SetName(std::string_view name){
            logicalDevice.SetObjectName(vkSemaphore, VK_OBJECT_TYPE_SEMAPHORE, name);
        }
#endif

}