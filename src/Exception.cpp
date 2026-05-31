#include "EightWinds/Backend/Exception.h"

#if EWE_USING_EXCEPTIONS
#if EWE_CALL_STACK_DEBUG
#include <stacktrace>
#endif
#endif

namespace EWE{
#if EWE_USING_EXCEPTIONS
        EWEException::EWEException(uint8_t skip, uint8_t max_depth, VkResult _result, std::string_view _msg) noexcept
        : std::runtime_error("EWE except"), 
            result{_result},
            msg{_msg}
#if EWE_CALL_STACK_DEBUG
            , stacktrace{std::stacktrace::current(skip, max_depth)}
#endif
        {}
        EWEException::EWEException(uint8_t skip, uint8_t max_depth, std::string_view _msg) noexcept
        : EWEException(skip, max_depth, VK_RESULT_MAX_ENUM, _msg)
        {}
        EWEException::EWEException(uint8_t skip, uint8_t max_depth, VkResult _result) noexcept
        : EWEException(skip, max_depth, _result, "")
        {}

        EWEException::EWEException(VkResult _result, std::string_view _msg) noexcept 
        : EWEException(1, 16, _result, _msg)
        {}

        EWEException::EWEException(std::string_view _msg) noexcept 
        : EWEException(1, 16, VK_RESULT_MAX_ENUM, _msg)
        {}

        EWEException::EWEException(VkResult _result) noexcept 
        : EWEException(1, 16, _result, "")
        {}
#endif
}//namespace EWE