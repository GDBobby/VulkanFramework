#pragma once


#include "EightWinds/Preprocessor.h"
#include "vulkan/vulkan.h"

#if EWE_CALL_STACK_DEBUG
#include <stacktrace>
#endif
#if EWE_USING_EXCEPTIONS
#include <cstdint>
#include <string_view>
#include <stdexcept>
#endif

namespace EWE{

#if EWE_USING_EXCEPTIONS
//im thinking i might need another level of inheritance for these exceptions
//one for vulkan related errors, that would be handled internally by the framework
//another for framework level errors, this is like missing required extensions
//potentially engine/game level errors on top of that
//im not really sure yet, i need to let this evolve from where its at
#define EWE_EXCEPT

    struct EWEException : public std::runtime_error {
        VkResult result;
        std::string msg;
        
        //this will be populated in construction
#if EWE_CALL_STACK_DEBUG
        std::stacktrace stacktrace; 
#endif
        [[nodiscard]] explicit EWEException(uint8_t skip, uint8_t max_depth, VkResult result, std::string_view msg) noexcept;
        [[nodiscard]] explicit EWEException(uint8_t skip, uint8_t max_depth, std::string_view msg) noexcept;
        [[nodiscard]] explicit EWEException(uint8_t skip, uint8_t max_depth, VkResult result) noexcept;
        [[nodiscard]] explicit EWEException(VkResult result, std::string_view msg) noexcept;
        [[nodiscard]] explicit EWEException(std::string_view msg) noexcept;
        [[nodiscard]] explicit EWEException(VkResult result) noexcept;
    };
#else
#define EWE_EXCEPT noexcept
#endif
}