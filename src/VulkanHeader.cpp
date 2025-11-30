#include "EightWinds/VulkanHeader.h"

#include <cassert>

namespace EWE{
    //i think im going to change this to print out some history of the render graph
    //that would mean castign a type erased pointer (void*), along with a enum val (VK_OBJECT_TYPE similar)
    //then including basically EVERYTHING in a cpp file
    //then interpreting the type, and printing a debug log

    
#if EWE_USING_EXCEPTIONS

#if CALL_TRACING
        EWEException::EWEException(uint8_t skip, uint8_t max_depth, VkResult result, std::string_view msg) noexcept
            :
            std::runtime_error("EWE except"), 
            result{result},
            text{msg},
            stacktrace{std::stacktrace::current(skip, max_depth)}
        {}
        EWEException::EWEException(uint8_t skip, uint8_t max_depth, std::string_view msg) noexcept
            : EWEException(skip, max_depth, VK_RESULT_MAX_ENUM, msg)
        {}
        EWEException::EWEException(uint8_t skip, uint8_t max_depth, VkResult result) noexcept
            : EWEException(skip, max_depth, result, "")
        {}
        
#endif

        EWEException::EWEException(VkResult result, std::string_view msg) noexcept : 
#if CALL_TRACING
            EWEException(1, 16, result, msg)
#else
            std::runtime_error(std::to_string(result)),
            result{result}, msg{msg}
#endif
        {}

        EWEException::EWEException(std::string_view msg) noexcept : 
#if CALL_TRACING
            EWEException(1, 16, VK_RESULT_MAX_ENUM, msg)
#else
            EWEException(VK_RESULT_MAX_ENUM, msg)
#endif
        {}

        EWEException::EWEException(VkResult result) noexcept : 
#if CALL_TRACING
            EWEException(1, 16, result, msg)
#else
            EWEException(result, "")
#endif
        {}


#endif
}