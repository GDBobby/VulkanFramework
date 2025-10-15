#include "vulkan/vulkan.hpp"

#include "Preprocessor.h"

#include <functional>
#include <type_traits>

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