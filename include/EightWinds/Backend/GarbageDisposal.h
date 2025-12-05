#pragma once

/*
im getting stunlocked, i need to put this off

#include "EightWinds/VulkanHeader.h"

#include "EightWinds/LogicalDevice.h"

namespace EWE{
    namespace Backend{

        template<typename T, typename... Ts>
        struct index_of;

        template<typename T>
        struct index_of<T> : std::integral_constant<int, -1> {};

        template<typename T, typename First, typename... Rest>
        struct index_of<T, First, Rest...> {
        private:
            static constexpr int next = index_of<T, Rest...>::value;
        public:
            static constexpr int value = std::is_same_v<T, First> ? 0 : (next == -1 ? -1 : 1 + next);
        };

        template<typename R, typename... Args>
        struct Vulkan_Function_Traits<R(*)(Args...)> {
            using args_tuple = std::tuple<Args...>;
            using Return_Type = R;

            static constexpr int logical_device_index = index_of<VkDevice, Args...>::value;
            static constexpr bool has_logical_device  = (logical_device_index != -1);

            static constexpr int physical_device_index = index_of<VkPhysicalDevice, Args...>::value;
            static constexpr bool has_physical_device  = (physical_device_index != -1);

            static constexpr int instance_index = index_of<VkInstance, Args...>::value;
            static constexpr bool has_instance  = (instance_index != -1);
        };

        template<std::size_t Pos, typename T, typename Tuple, std::size_t... I1, std::size_t... I2>
        auto tuple_insert_impl(Tuple&& tup, T&& value, std::index_sequence<I1...>, std::index_sequence<I2...>) {
            return std::tuple_cat(
                std::make_tuple(std::get<I1>(std::forward<Tuple>(tup))...),
                std::make_tuple(std::forward<T>(value)),
                std::make_tuple(std::get<Pos + 1 + I2>(std::forward<Tuple>(tup))...)
            );
        }

        template<std::size_t Pos, typename T, typename Tuple>
        auto tuple_insert(Tuple&& tup, T&& value) {
            constexpr size_t N = std::tuple_size<std::remove_reference_t<Tuple>>::value;
            static_assert(Pos <= N);
            return tuple_insert_impl<Pos>(
                std::forward<Tuple>(tup), std::forward<T>(value),
                std::make_index_sequence<Pos>{},
                std::make_index_sequence<N - Pos>{}
            );
        }

        struct GarbageDisposal{
            LogicalDevice& logicalDevice;

            template<typename DestroyFunc, typename... Args>
            auto Populate(Args&&... args) {
                using vk_traits = Vulkan_Function_Traits<DestroyFunc>;

                static_assert(traits::logical_has_device,
                            "Destroy function must accept a VkDevice");

                constexpr int device_index = traits::logical_device_index;

                auto base_tup = std::make_tuple(std::forward<Args>(args)...);

                DestroyFunc func = nullptr;
                if constexpr(vk_traits::has_logical_device) {

                    auto final_args = tuple_insert<device_index>(base_tup, device);

                    return [t = std::move(final_args), func]() mutable {
                        std::apply(
                            [&](auto&&... packed_args){
                                func(std::forward<decltype(packed_args)>(packed_args)...);
                            },
                            t
                        );
                        return true;
                    };
                }
            }
        };
    } //namespace backend
} //namespace EWE
*/