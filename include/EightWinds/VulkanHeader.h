#pragma once
#include "vulkan/vulkan.h"

#include "Preprocessor.h"

#include <functional>
#include <type_traits>
#include <concepts>

#if EWE_USING_EXCEPTIONS
#include <stdexcept>
#endif

#ifdef _WIN32
//is this for vma? i dont remember
#define WIN32_LEAN_AND_MEAN
#endif

#include "vma/include/vk_mem_alloc.h"

#if EWE_CALL_STACK_DEBUG
#include <stacktrace>
#endif

//i dont want anything in here besides global VULKAN related functions,
//typeids, like PipelineID,
//and vk result assertion (EWE_VK)

namespace EWE{

    
    static constexpr uint8_t max_frames_in_flight = 2;

    using PipelineID = uint64_t;


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
        
#if EWE_CALL_STACK_DEBUG
        //this will be populated in construction
        std::stacktrace stacktrace; 
        [[nodiscard]] explicit EWEException(uint8_t skip, uint8_t max_depth, VkResult result, std::string_view msg) noexcept;
        [[nodiscard]] explicit EWEException(uint8_t skip, uint8_t max_depth, std::string_view msg) noexcept;
        [[nodiscard]] explicit EWEException(uint8_t skip, uint8_t max_depth, VkResult result) noexcept;
#endif
        [[nodiscard]] explicit EWEException(VkResult result, std::string_view msg) noexcept;
        [[nodiscard]] explicit EWEException(std::string_view msg) noexcept;
        [[nodiscard]] explicit EWEException(VkResult result) noexcept;
    };
#else
#define EWE_EXCEPT noexcept
#endif

    constexpr uint32_t RoundDownVkVersion(uint32_t in_version) noexcept {
        constexpr uint32_t mask = (1 << 12) - 1;
        constexpr uint32_t inverted_mask = ~mask;
        return in_version & inverted_mask;
    }
    template <typename T>
    concept VulkanStruct = requires(T t) {
        { t.sType } -> std::convertible_to<VkStructureType>;
        { t.pNext } -> std::convertible_to<const void*>;
    };

    void EWE_VK_RESULT(VkResult vkResult);

    template<typename F, typename... Args>
    requires (std::is_invocable_v<F, Args...>)
    inline constexpr void EWE_VK(F&& f, Args&&... args) {
    #if WRAPPING_VULKAN_FUNCTIONS
        //call a preliminary function
    #endif
        EWE_VK_RESULT(std::invoke(std::forward<F>(f), std::forward<Args>(args)...));
        
    #if WRAPPING_VULKAN_FUNCTIONS
        //call a following function
    #endif
    }



    constexpr bool GetAccessMaskWrite(VkAccessFlagBits2 accessMask) {
        switch (accessMask) {
            case VK_ACCESS_2_SHADER_WRITE_BIT:
            case VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT:
            case VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT:
            case VK_ACCESS_2_TRANSFER_WRITE_BIT:
            case VK_ACCESS_2_HOST_WRITE_BIT:
            case VK_ACCESS_2_MEMORY_WRITE_BIT:
            case VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT:
            case VK_ACCESS_2_VIDEO_DECODE_WRITE_BIT_KHR:
            case VK_ACCESS_2_VIDEO_ENCODE_WRITE_BIT_KHR:
            case VK_ACCESS_2_SHADER_TILE_ATTACHMENT_WRITE_BIT_QCOM:
            case VK_ACCESS_2_TRANSFORM_FEEDBACK_WRITE_BIT_EXT:
            case VK_ACCESS_2_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT:
            case VK_ACCESS_2_COMMAND_PREPROCESS_WRITE_BIT_EXT:
            case VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR:
            case VK_ACCESS_2_MICROMAP_WRITE_BIT_EXT:
            case VK_ACCESS_2_OPTICAL_FLOW_WRITE_BIT_NV:
            case VK_ACCESS_2_DATA_GRAPH_WRITE_BIT_ARM:
                return true;
            default:
                return false;
        }
    }
}