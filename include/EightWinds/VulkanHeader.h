#pragma once
#include "vulkan/vulkan.h"

#include "Preprocessor.h"

#include <functional>
#include <type_traits>
#include <concepts>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#endif

#include "vma/include/vk_mem_alloc.h"

namespace EWE{

    void EWE_VK_RESULT(VkResult vkResult);

    template<typename F, typename... Args>
    requires (std::is_invocable_v<F, Args...>)
    constexpr VkResult EWE_VK(F&& f, Args&&... args) {
    #if WRAPPING_VULKAN_FUNCTIONS
        //call a preliminary function
    #endif
#if DEBUGGING_DEVICE_LOST                
        VkResult vkResult = std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
        if (vkResult != VK_SUCCESS) {                                                                        
            if (vkResult == VK_ERROR_DEVICE_LOST) { EWE::VKDEBUG::OnDeviceLost(); }
        }
        return vkResult;
#else
        return std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
#endif
        
    #if WRAPPING_VULKAN_FUNCTIONS
        //call a following function
    #endif
    }
}