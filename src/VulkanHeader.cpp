#include "EightWinds/VulkanHeader.h"

#include <cassert>

namespace EWE{
    //i think im going to change this to print out some history of the render graph

    #if CALL_TRACING
    void EWE_VK_RESULT(VkResult vkResult) {
    #if DEBUGGING_DEVICE_LOST                                                                                        
        if (vkResult == VK_ERROR_DEVICE_LOST) { EWE::VKDEBUG::OnDeviceLost(); }
        else
    #endif
        if (vkResult != VK_SUCCESS) {
        
            printf("need to set up stack tracing here\n");
    #if COMMAND_BUFFER_TRACING
            for (uint8_t i = 0; i < EWE::vkObject->renderCommands.size(); i++) {
                while (EWE::vkObject->renderCommands[i].usageTracking.size() > 0) {
                    for (auto& usage : EWE::vkObject->renderCommands[i].usageTracking.front()) {
                        logFile << "cb(" << +i  << ")(" << EWE::vkObject->renderCommands[i].usageTracking.size() << ") : " << usage.funcName << '\n';
                    }
                    EWE::vkObject->renderCommands[i].usageTracking.pop();
                }
            }
    #endif
            assert(vkResult == VK_SUCCESS && "VK_ERROR");
        }
    }
    #else
    void EWE_VK_RESULT(VkResult vkResult) {
#if DEBUGGING_DEVICE_LOST                                                                                        
        if (vkResult == VK_ERROR_DEVICE_LOST) { EWE::VKDEBUG::OnDeviceLost(); }
#endif
        assert(vkResult == VK_SUCCESS);
    }
    #endif
}