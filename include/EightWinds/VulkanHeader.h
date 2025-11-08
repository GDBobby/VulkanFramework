#pragma once
#include "vulkan/vulkan.h"

#include "Preprocessor.h"

#include <functional>
#include <type_traits>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#endif
#include "vma/include/vk_mem_alloc.h"

namespace EWE{

    void EWE_VK_RESULT(VkResult vkResult);

    template<typename F, typename... Args>
    auto EWE_VK(F&& f, Args&&... args) {
    #if WRAPPING_VULKAN_FUNCTIONS
        //call a preliminary function
    #endif
        if constexpr (std::is_void_v<decltype(std::forward<F>(f)(std::forward<Args>(args)...))>) {
            std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
        }
        else {
            VkResult vkResult = std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
            if (vkResult != VK_SUCCESS) {
    #if DEBUGGING_DEVICE_LOST                                                                                        
                if (vkResult == VK_ERROR_DEVICE_LOST) { EWE::VKDEBUG::OnDeviceLost(); }
    #endif
            }
            return vkResult;
        }
    #if WRAPPING_VULKAN_FUNCTIONS
        //call a following function
    #endif
    }
}