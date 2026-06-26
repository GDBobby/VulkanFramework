#pragma once
#include "vulkan/vulkan.h"

#include "EightWinds/Preprocessor.h"
#include "EightWinds/Backend/Exception.h"

#include <functional>
#include <type_traits>
#include <concepts>


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
    template <typename T>
    requires(std::integral<T> || std::floating_point<T>)
    static constexpr T DEFAULT_HEIGHT = T(1080);
    template <typename T>
    requires(std::integral<T> || std::floating_point<T>)
    static constexpr T DEFAULT_WIDTH = T(1920);


    #define for_each_frame for(uint8_t frame = 0; frame < max_frames_in_flight; frame++)

    using PipelineID = uint64_t;

    static constexpr int null_texture = -1;
    static constexpr VkDeviceAddress null_buffer = 0;

    struct DeviceAddress {
        VkDeviceAddress value;

        // Optional: implicit conversions so it behaves like an integer in your code
        DeviceAddress() : value(null_buffer) {}
        /*implicit*/ DeviceAddress(VkDeviceAddress val) : value(val) {}
        operator VkDeviceAddress() const { return value; }
    };
    struct TextureIndex {
        int value;
        TextureIndex() : value(null_texture) {}
        TextureIndex(int val) : value(val) {}
        operator int() const { return value; }
    };


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


    //constexpr std::size_t EWE_MAGIC_VALUE = 0x40000000004;

    //template<typename T>
    //constexpr bool ewe_castable_to_size_t =
    //    std::is_pointer_v<std::remove_reference_t<T>> ||
    //    std::is_integral_v<std::remove_reference_t<T>> ||
    //    std::is_enum_v<std::remove_reference_t<T>>;

    //template<typename T>
    //inline bool ewe_arg_matches_magic(T&& arg) {
    //    using U = std::remove_reference_t<T>;

    //    if constexpr (std::is_pointer_v<U>) {
    //        return reinterpret_cast<std::size_t>(arg) == EWE_MAGIC_VALUE;
    //    }
    //    else if constexpr (std::is_integral_v<U> || std::is_enum_v<U>) {
    //        return static_cast<std::size_t>(arg) == EWE_MAGIC_VALUE;
    //    }
    //    else {
    //        return false;
    //    }
    //}

    template<typename F, typename... Args>
    requires (std::is_invocable_v<F, Args...>)
    inline constexpr void EWE_VK(F&& f, Args&&... args) {
    #if WRAPPING_VULKAN_FUNCTIONS
        //call a preliminary function
        //if constexpr ((ewe_castable_to_size_t<Args> || ...)) {
        //    if ((ewe_arg_matches_magic(args) || ...)) {
        //        Log::Warning("pause\n");
        //    }
        //}
    #endif


        EWE_VK_RESULT(std::invoke(std::forward<F>(f), std::forward<Args>(args)...));
        
    #if WRAPPING_VULKAN_FUNCTIONS
        //call a following function
    #endif
    }

    void NameCurrentThread(std::string_view name);
}