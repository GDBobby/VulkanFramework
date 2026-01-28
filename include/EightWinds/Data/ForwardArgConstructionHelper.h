#pragma once

#include <cstdint>
#include <concepts>
#include <memory>

namespace EWE{
	
    template<std::size_t Begin, std::size_t Length, typename Tuple, std::size_t... Is>
    auto tuple_slice_impl(Tuple&& tup, std::index_sequence<Is...>) {
        return std::forward_as_tuple(std::get<Begin + Is>(std::forward<Tuple>(tup))...);
    }

    template<std::size_t Begin, std::size_t Length, typename Tuple>
    auto tuple_slice_index_creator(Tuple&& tup) {
        return tuple_slice_impl<Begin, Length>(
            std::forward<Tuple>(tup),
            std::make_index_sequence<Length>{}
        );
    }

    template<std::size_t Begin, std::size_t Length, typename T, typename... Args>
    static void ConstructFrom_ForwardedArgumentPackSlice(T* construction_address, Args&&... args) {
        auto forwarded_as_tuple = std::forward_as_tuple(std::forward<Args>(args)...);

        auto sliced_tuple = tuple_slice_index_creator<Begin, Length>(forwarded_as_tuple);

        std::apply(
            [construction_address](auto&&... slice_args) {
                std::construct_at(
                    construction_address,
                    std::forward<decltype(slice_args)>(slice_args)...
                );
            },
            sliced_tuple
        );
    }
    template<std::size_t Begin, std::size_t Length, typename T, typename... Args>
    static T ConstructFrom_ForwardedArgumentPackSlice(Args&&... args) {
        auto forwarded_as_tuple = std::forward_as_tuple(std::forward<Args>(args)...);

        auto sliced_tuple = tuple_slice_index_creator<Begin, Length>(forwarded_as_tuple);

        return std::apply(
            [](auto&&... slice_args) -> T {
                return T{ std::forward<decltype(slice_args)>(slice_args)... };
            },
            sliced_tuple
        );
    }
	
	template<std::size_t... Offsets>
	struct ArgumentPack_ConstructionHelper {
		static constexpr std::size_t offsets[sizeof...(Offsets)] = {Offsets...};
		static constexpr std::size_t object_count = sizeof...(Offsets) + 1;
		
		template<typename T, typename... Args>
		static void ConstructContiguous(T* uninitialized_memory, Args&&... args){
			
			static constexpr std::size_t offset_count = sizeof...(Offsets);
			
			for(std::size_t i = 0; i < (offset_count - 1); i++){
				ConstructFrom_ForwardedArgumentPackSlice<offsets[i], offsets[i + 1], T, Args...>(uninitialized_memory + i, std::forward<Args>(args)...);
			}
			
			static constexpr std::size_t total_arg_count = sizeof...(Args);
			ConstructFrom_ForwardedArgumentPackSlice<offsets[offset_count - 1], total_arg_count, T, Args...>(uninitialized_memory + offset_count, std::forward<Args>(args)...);
		}
	};

    struct OnlyConstructOneObject_Helper {
        //wild that i need this
    };
	
}