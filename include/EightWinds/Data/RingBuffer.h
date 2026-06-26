#pragma once

#include "EightWinds/Data/StackBlock.h"

#include "EightWinds/Preprocessor.h"

#include <bitset>
#include <cstdint>
#include <concepts>
#include <thread>

#if EWE_DEBUG_BOOL
#include <stacktrace>
#endif

//somewhat specialized
namespace EWE{
	template<typename T, std::size_t _Size>
	struct RingBuffer{
		static constexpr std::size_t Size = _Size;
		
		StackBlock<T, Size> data;
		std::bitset<Size> usage;
#if EWE_DEBUG_BOOL
		std::array<std::stacktrace, Size> usage_location;
#endif
		
		std::size_t starting_index = 0;
		
		template<typename... Args>
		requires(std::is_constructible_v<T, Args...>)
		[[nodiscard]] RingBuffer(Args&&... args) {
			data.ConstructAll(std::forward<Args>(args)...);
		}

		[[nodiscard]] RingBuffer()
			requires(std::is_trivially_constructible_v<T>)
		{}

		T* GetNext(){
			std::size_t current_index = starting_index;
			while(usage[current_index]){
				current_index = (current_index + 1) % Size;
				EWE_ASSERT(current_index != starting_index);
				if(current_index == starting_index){
					//std::this_thread::sleep_for(std::chrono::nanoseconds(1));
				}
			}
			starting_index = (current_index + 1) % Size;
#if EWE_DEBUG_BOOL
			usage_location[current_index] = std::stacktrace::current();
#endif
			usage[current_index] = true;
			return &data[current_index];
		}
		void Return(T* obj){
			std::size_t i = obj - &data[0];
			EWE_ASSERT(i < Size && usage[i]);
			usage[i] = false;
#if EWE_DEBUG_BOOL
			usage_location[i] = std::stacktrace{};
#endif
		}

		bool Full() const{
			return usage.count() == Size;
		}

		auto begin() {return data.begin();}
		auto end() {return data.end();}
	private:
		
		void UpdateIndex(){
			starting_index = (starting_index + 1) % Size;
		}
	};
}