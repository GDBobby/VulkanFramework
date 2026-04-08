#pragma once

#include <cstdint>
#include <concepts>
#include <bitset>
#include <memory>

#include "EightWinds/Data/MemoryHelpers.h"

#include <cstring> //memcpy

//UNFINISHED

namespace EWE{
	
	
    template<typename T>
    struct HeapBlock : public MemoryHelper_Construction<HeapBlock<T>, T> {
	private:
        std::allocator<T> allocator{};
	public:
        T* memory;

		//DO NOT CHANGE EXTERNALLY
		std::size_t size; //DO NOT CHANGE EXTERNALLY

	public:
	
		[[nodiscard]] HeapBlock() 
		: memory{nullptr}, size{0}
		{}

        [[nodiscard]] explicit HeapBlock(std::size_t _size) 
		: size{_size}
		{
			if(size == 0){
				memory = nullptr;	
			}
			else{
            	memory = allocator.allocate(size);
			}
        }
		~HeapBlock(){
			if(size != 0){
				allocator.deallocate(memory, size);
			}
		}

		
		[[nodiscard]] HeapBlock(HeapBlock const& copySrc)
		requires std::is_trivially_copyable_v<T>
		: memory{allocator.allocate(copySrc.size)},
			size{copySrc.size}
		{
			memcpy(memory, copySrc.memory, size * sizeof(T));
		}
		HeapBlock& operator=(HeapBlock& copySrc) = delete;
		HeapBlock(HeapBlock&& moveSrc) noexcept
			: memory{moveSrc.memory},
			size{moveSrc.size}
		{
			moveSrc.memory = nullptr;
			moveSrc.size = 0;
		}

		HeapBlock& operator=(HeapBlock&& moveSrc) noexcept = delete;

		void Clear(){
			if(size != 0){
				allocator.deallocate(memory, size);
			}
			size = 0;
			memory = nullptr;
		}
		
		void Resize(std::size_t count){
			if(count != size){
				if(size != 0){
					allocator.deallocate(memory, count);
				}
				memory = allocator.allocate(count);
				size = count;
			}
		}
    };
	
	//this wont control construction or deconstruction, just the memory itself
}