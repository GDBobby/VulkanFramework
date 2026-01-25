#pragma once

#include <cstdint>
#include <type_traits>
#include <memory>

#include "EightWinds/Data/ForwardArgConstructionHelper.h"

#include "EightWinds/Data/HeapBlock.h"

namespace EWE{
	/*
		the purpose of this container is to have a variable size, like the vector
		but to not invoke move/copy semantics
		
		the size is determined at construction, based on runtime variables
		
		if size is known at compile time, use std::array
		if resize is required, use std::vector
		if resize/insert/removal is required, without move/copy semantics, use EightWinds/Data/Hive

		this uses heap memory
	*/
	
	template<typename T>//, bool AllocateAtLeast = true> 
	struct RuntimeArray{
		//low priority -
		//  use template sfinae or some other thing to abstract allocator::allocate and allocaotr::allocate_at_least

		HeapBlock<T> heap;

	public:

		std::size_t Size() const {
			return heap.Size();
		}
		T* Data() {
			return heap.GetMemory();
		}
		T const* Data() const {
			return heap.GetMemory();
		}

		void ClearAndResize(std::size_t elementCount) {
			heap.DestroyAll();
			heap.Resize(elementCount);
			heap.ConstructAll();
		}

		template<typename... Args>
			requires std::is_constructible_v<T, Args...>
		void ClearAndResize(std::size_t elementCount, Args&&... args) {
			heap.DestroyAll();
			heap.Resize(elementCount);
			heap.ConstructAll(std::forward<Args>(args)...);
		}

		template<std::size_t... Offsets, typename... Args>
		void ClearAndResize(ArgumentPack_ConstructionHelper<Offsets...> helper, Args&&... args) {
			heap.DestroyAll();
			heap.Resize(helper.object_count);
			heap.ConstructAll(helper, std::forward<Args>(args)...);
		}

		[[nodiscard]] explicit RuntimeArray(std::size_t elementCount)
			requires std::is_default_constructible_v<T>
			: heap{elementCount}
		{
#if EWE_DEBUG_BOOL
			assert(elementCount > 0);
#endif
			heap.ConstructAll();
		}
		
		template<typename... Args>
			requires std::is_constructible_v<T, Args...>
		[[nodiscard]] explicit RuntimeArray(std::size_t elementCount, Args&&... args)
			: heap{elementCount}
		{
#if EWE_DEBUG_BOOL
			assert(elementCount > 0);
#endif
			heap.ConstructAll(std::forward<Args>(args)...);
		}
		
		//we would prefer if helper got optimized away. potentially worth taking a micro-optimization glance at
		template<std::size_t... Offsets, typename... Args>
		[[nodiscard]] explicit RuntimeArray(ArgumentPack_ConstructionHelper<Offsets...> helper, Args&&... args)
		: heap{ ArgumentPack_ConstructionHelper<Offsets>::object_count}
		{
			heap.ConstructAll(helper, std::forward<Args>(args)...);
		}

		//this could be copying, it could be conversion/inheritance (like how imageview can be constructed only from image)
		template<typename Other>
			requires std::is_constructible_v<T, Other&>
		[[nodiscard]] explicit RuntimeArray(RuntimeArray<Other>& other)
			: heap(other.heap.size)
		{
			for (std::size_t i = 0; i < heap.size; ++i) {
				std::construct_at(
					heap.GetMemory() + i,
					other.heap[i]
				);
			}
		}
		
		~RuntimeArray() { heap.DestroyAll(); }
		
		RuntimeArray(RuntimeArray const& copySrc) = delete;
		RuntimeArray& operator=(RuntimeArray const& copySrc) = delete;
		
		RuntimeArray(RuntimeArray&& moveSrc) noexcept
			: heap{ moveSrc.heap }
		{}

		RuntimeArray& operator=(RuntimeArray&& moveSrc) noexcept = delete;
		
        T& operator[](size_t i) noexcept { return heap[i]; }
        T const& operator[](size_t i) const noexcept { return heap[i]; }
		
		T* begin() noexcept {return heap.begin();}
		T* end() noexcept {return heap.end();}
		
		T const* cbegin() const noexcept {return heap.cbegin();}
		T* cend() const noexcept {return heap.cend();}
		T const* begin() const noexcept {return heap.begin();}
		T* end() const noexcept {return heap.end();}
	};
}