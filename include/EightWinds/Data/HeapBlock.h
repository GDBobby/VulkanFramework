#pragma once

#include <cstdint>
#include <concepts>
#include <bitset>
#include <memory>

#include "EightWinds/Data/MemoryHelpers.h"

//UNFINISHED

namespace EWE{
	
	
    template<typename T>
    struct HeapBlock : public MemoryHelper_Construction<HeapBlock<T>, T> {
        T* memory;

		//DO NOT CHANGE EXTERNALLY
		std::size_t size; //DO NOT CHANGE EXTERNALLY
	private:
        std::allocator<T> allocator{};

	public:
	
		[[nodiscard]] HeapBlock() 
		: memory{nullptr}, size{0}
		
		{}

        [[nodiscard]] explicit HeapBlock(std::size_t size) : size{size}{
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

		HeapBlock(HeapBlock& copySrc) = delete;
		HeapBlock& operator=(HeapBlock& copySrc) = delete;
		HeapBlock(HeapBlock&& moveSrc) noexcept
			: memory{moveSrc.memory},
			size{moveSrc.memory}
		{
			moveSrc.memory = nullptr;
			moveSrc.size = 0;
		}
		HeapBlock& operator=(HeapBlock&& moveSrc) noexcept = delete;
		
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