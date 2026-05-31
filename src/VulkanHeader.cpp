#include "EightWinds/VulkanHeader.h"

#include "EightWinds/Reflect/Enum.h"

namespace EWE{
    //i think im going to change this to print out some history of the render graph
    //that would mean castign a type erased pointer (void*), along with a enum val (VK_OBJECT_TYPE similar)
    //then including basically EVERYTHING in a cpp file
    //then interpreting the type, and printing a debug log

        void EWE_VK_RESULT(VkResult vkResult) {
            if (vkResult != VK_SUCCESS) {
#if EWE_DEBUG_BOOL
                if (vkResult != VK_ERROR_DEVICE_LOST) {
                    //going to use the device lost extension here
                    EWE_ASSERT(vkResult == VK_SUCCESS, Reflect::Enum::ToString(vkResult));

                //the callstack isnt relevant for device lost
#endif
#if EWE_CALL_STACK_DEBUG
                    std::stacktrace error_trace = std::stacktrace::current(2);
                    std::cout << "vk result : " << vkResult << std::endl << error_trace << std::endl << std::endl;
                    std::this_thread::sleep_for(std::chrono::seconds(5));
#endif
#if EWE_DEBUG_BOOL
                }
#endif
#if EWE_USING_EXCEPTIONS
                throw EWEException(vkResult);
#endif
            }
        }
}