#include "EightWinds/Backend/Semaphore.h"

namespace EWE{
    BinarySemaphore::BinarySemaphore(LogicalDevice& _logicalDevice)
        :logicalDevice{_logicalDevice}
    {
        VkSemaphoreCreateInfo semaphoreCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0
        };
        EWE_VK(vkCreateSemaphore, logicalDevice.device, &semaphoreCreateInfo, nullptr, &vkSemaphore);
    }
    BinarySemaphore::~BinarySemaphore() {
        if (vkSemaphore != VK_NULL_HANDLE) {
            logicalDevice.garbageDisposal.Toss(vkSemaphore, VK_OBJECT_TYPE_SEMAPHORE);
        }
    }
    
    BinarySemaphore::BinarySemaphore(BinarySemaphore&& moveSrc) noexcept
        : logicalDevice{moveSrc.logicalDevice},
        vkSemaphore{moveSrc.vkSemaphore}//,
        //waiting{moveSrc.waiting},
        //signaling{moveSrc.signaling}
    {
        moveSrc.vkSemaphore = VK_NULL_HANDLE;
    }
    BinarySemaphore& BinarySemaphore::operator=(BinarySemaphore&& moveSrc) noexcept{
        EWE_ASSERT(logicalDevice == moveSrc.logicalDevice);
        vkSemaphore = moveSrc.vkSemaphore;
        moveSrc.vkSemaphore = VK_NULL_HANDLE;

        //waiting = moveSrc.waiting;
        //signaling = moveSrc.signaling;
        return *this;
    }
    
#if EWE_DEBUG_NAMING
        void BinarySemaphore::SetName(std::string_view name){
            logicalDevice.SetObjectName(vkSemaphore, VK_OBJECT_TYPE_SEMAPHORE, name);
        }
#endif







    TimelineSemaphore::TimelineSemaphore(LogicalDevice& _logicalDevice, uint64_t initialValue)
        : logicalDevice{_logicalDevice}
    {
        //VK_SEMAPHORE_TYPE_BINARY = 0,
        //VK_SEMAPHORE_TYPE_TIMELINE = 1,

        //i need to find a way to rectify features internally
#if EWE_DEBUG_BOOL
        EWE_ASSERT(logicalDevice.features12.timelineSemaphore);
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
        EWE_ASSERT(logicalDevice == moveSrc.logicalDevice);
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


    VkSemaphoreSubmitInfo TimelineSemaphore::GetSignalSubmitInfo(VkPipelineStageFlags2 stageMask) noexcept{
        value++;
#if EWE_DEBUG_BOOL
        Logger::Print("semaphore[%s] subinfo signaling : %zu\n", debugName.c_str(), value);
#endif
        return VkSemaphoreSubmitInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .pNext = nullptr,
            .semaphore = vkSemaphore,
            .value = value,
            .stageMask = stageMask,
            .deviceIndex = 0
        };
    }
    VkSemaphoreSubmitInfo TimelineSemaphore::GetWaitSubmitInfo(VkPipelineStageFlags2 stageMask) const noexcept{
#if EWE_DEBUG_BOOL
        Logger::Print("semaphore[%s] subinfo waiting : %zu\n", debugName.c_str(), value);
#endif
        return VkSemaphoreSubmitInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .pNext = nullptr,
            .semaphore = vkSemaphore,
            .value = value,
            .stageMask = stageMask,
            .deviceIndex = 0
        };
    }
    VkSemaphoreSubmitInfo TimelineSemaphore::GetSubmitInfo(VkPipelineStageFlags2 stageMask, bool signal) noexcept{
        if(signal){
            return GetSignalSubmitInfo(stageMask);
        }
        return GetWaitSubmitInfo(stageMask);
    }
    
    void TimelineSemaphore::WaitOn(uint64_t val){
#if EWE_DEBUG_BOOL
        Logger::Print("semaphore[%s] waiting : %zu\n", debugName.c_str(), value);
#endif
        VkSemaphoreWaitInfo waitInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
            .pNext = nullptr,
            .flags = 0,
            .semaphoreCount = 1,
            .pSemaphores = &vkSemaphore,
            .pValues = &val
        };
        
        //timeout isn't acceptable, 2s is when the failure will be reported
        EWE_VK(vkWaitSemaphores, logicalDevice, &waitInfo, std::chrono::nanoseconds(std::chrono::seconds(2)).count());//UINT64_MAX);
    }
    uint64_t TimelineSemaphore::GetCurrentValue() const {
        uint64_t active_val;
        EWE_VK(vkGetSemaphoreCounterValue, logicalDevice, vkSemaphore, &active_val);
        return active_val;
    }
    bool TimelineSemaphore::Check(uint64_t val) const {
        uint64_t current_val = GetCurrentValue();
#if EWE_DEBUG_BOOL
        Logger::Print("semaphore[%s] checking : %zu : [%zu : %zu]\n", debugName.c_str(), value, val, current_val);
#endif
        if(current_val >= val){
            //potentially check current_val <-> this->value?
            return true;
        }
        return false;
    }

}