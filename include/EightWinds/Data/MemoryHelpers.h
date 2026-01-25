#pragma once

#include "EightWinds/Data/ForwardArgConstructionHelper.h"

#include <cstdint>
#include <concepts>
#include <type_traits>

namespace EWE{
	

	template<typename Memory, typename T>
	struct MemoryHelper{

		
		constexpr Memory& CastSelf(){
			return *static_cast<Memory*>(this);
		}
		constexpr Memory const& CastSelf() const {
			return *static_cast<Memory const*>(this);
		}
        std::size_t Size() const {
            return CastSelf().size;
        }

        constexpr T* GetMemory() {
            auto& self = CastSelf();
            using memory_type = std::remove_reference_t<decltype(self.memory)>;
            if constexpr (std::is_same_v<memory_type, T*>){
                return self.memory;
            }
            else{
                auto* castedMemory = reinterpret_cast<T*>(self.memory);
                return castedMemory;
            }
        }
        constexpr T const* GetMemory() const {
            auto& self = CastSelf();
            using memory_type = std::remove_reference_t<decltype(self.memory)>;
            if constexpr (std::is_same_v<memory_type, T*>){
                return self.memory;
            }
            else{
                auto* castedMemory = reinterpret_cast<T const*>(self.memory);
                return castedMemory;
            }
        }

        constexpr T& operator[](std::size_t i) { 
            return GetMemory()[i];
        }
        
        constexpr T const& operator[](std::size_t i) const { 
            return GetMemory()[i];
        }

        T* begin() noexcept { return GetMemory(); }
        T* end() noexcept { return GetMemory() + Size(); }

        T const* cbegin() const noexcept { return GetMemory();}
        T* cend() const noexcept { return GetMemory() + Size(); }
        T const* begin() const noexcept { return GetMemory(); }
        T* end() const noexcept { return GetMemory() + Size(); }
	};

    template<typename Memory, typename T>
    struct MemoryHelper_Construction : public MemoryHelper<Memory, T> {
        
        template<typename... Args>
        void ConstructAt(std::size_t index, Args&&... args){				
            std::construct_at(
                this->GetMemory() + index,
                std::forward<Args>(args)...
            );
        }
		
		template<typename... Args>
		void ConstructAll(Args&&... args) {
            auto& self = this->CastSelf();
			for (std::size_t i = 0; i < self.size; i++) {
				ConstructAt(i, std::forward<Args>(args)...);
			}
		}
		
		template<std::size_t... Offsets, typename... Args>
		void ConstructAll(ArgumentPack_ConstructionHelper<Offsets...> helper, Args&&... args) {
			using arg_helper = ArgumentPack_ConstructionHelper<Offsets>;
			arg_helper::ConstructContiguous(this->GetMemory());
		}

        void DestroyAt(std::size_t index){
            std::destroy_at(this->GetMemory() + index);
        }

        //going to be easy to double deconstruct with this
        //which is arguably worse than double constructing
        void DestroyAll(){
            auto& self = this->CastSelf();
            for(std::size_t i = 0; i < self.size; i++){
                DestroyAt(i);
            }
        }
    };

}