#include "EightWinds/Backend/Fence.h"

namespace EWE{
    
    bool Fence::CheckReturn(uint64_t time) {
        if (!submitted) {
#if DEBUGGING_FENCES
            log.push_back("checked return, not submitted");
#endif
            return false;
        }
#if DEBUGGING_FENCES
        log.push_back("checked return, submitted");
#endif

#if EWE_DEBUG_BOOL
        if(vkFence == VK_NULL_HANDLE){
            printf("invalid fence?\n");
        }
#endif
        VkResult ret = vkWaitForFences(logicalDevice.device, 1, &vkFence, true, time);
        if (ret == VK_SUCCESS) {
            EWE_VK(vkResetFences, logicalDevice.device, 1, &vkFence);
            //its up to the calling function to unlock the mutex
            for (auto& waitSem : waitSemaphores) {
                waitSem->FinishWaiting();
            }
            waitSemaphores.clear();
            //makes more sense to clear the submitted flag here, rather than on acquire
            submitted = false;
            return true;
        }
        else if (ret == VK_TIMEOUT) {
            //its up to the calling function to unlock the mutex
            return false;
        }
        else {
            //its up to the calling function to unlock the mutex
            EWE_VK_RESULT(ret);
            return false; //error silencing, this should not be reached
        }
    }
}