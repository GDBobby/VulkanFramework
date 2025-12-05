#pragma once

#include "EightWinds/Preprocessor.h"

#include <cstdint>
#include <type_traits>
#include <concepts>
#include <memory>

#include <vector>
#include <bitset>

namespace EWE{
    //based on a very cursory glance at std::hive (P0447R21)
    template<typename T, uint8_t RowWidth = 16>
    struct Hive{
        static constexpr std::size_t RowSize = sizeof(T) * RowWidth;

        //uint8 just so i can move bytewise thru the memory
        struct Comb{
            uint8_t* memory;
            std::bitset<RowWidth> occupancy;

            [[nodiscard]] Comb()
            : memory{reinterpret_cast<uint8_t*>(malloc(RowSize))},
                occupancy{}
            {
                if (memory == nullptr) {
                    throw std::runtime_error("failed to malloc in hive");
                }
            }
            ~Comb(){
#if 0//EWE_DEBUG_BOOL //idk if i want this, maybe just destruct
                if(occupancy[rowIndex].any()){
                    throw std::invalid_argument("freeing objects that have not been destroyed");
                }
#else
                for(std::size_t i = 0; i < RowWidth; i++){
                    if(occupancy[i]){
                        std::destroy_at(reinterpret_cast<T*>(memory + (sizeof(T) * i)));
                    }
                }
#endif
                if(memory != nullptr){
                    free(memory);
                }
            }

            Comb(Comb const& copySrc) = delete;
            Comb& operator=(Comb const& copySrc) = delete;
            Comb& operator=(Comb&& moveSrc) = delete;

            Comb(Comb&& moveSrc) noexcept
            : memory{moveSrc.memory}, 
                occupancy{moveSrc.occupancy}
            {   
                moveSrc.memory = nullptr;
            }
        };
        std::vector<Comb> combs;

        [[nodiscard]] explicit Hive()
            : combs {}
        {
            AddRow();
        }

        T* GetAConstructionPointer(){
            for(auto& comb : combs){
                if(!comb.occupancy.all()){
                    for(std::size_t i = 0; i < RowWidth; i++){
                        if(!comb.occupancy[i]){
                            comb.occupancy.set(i, true);
                            return reinterpret_cast<T*>(comb.memory + (sizeof(T) * i));
                        }
                    }
                }
            }
            AddRow();
            auto& combBack = combs.back();
            combBack.occupancy.set(0, true);
            return reinterpret_cast<T*>(combBack.memory);
        }

        template<typename... Args>
            requires std::constructible_from<T, Args...>
        T& AddElement(Args&&... args){
            return *std::construct_at(GetAConstructionPointer(), std::forward<Args>(args)...);
        }

        void DestroyElement(T* element) {
            const std::size_t elementAddr = reinterpret_cast<std::size_t>(element);
            std::destroy_at<T>(element);

            for(auto& comb : combs){
                const std::size_t comb_begin = reinterpret_cast<std::size_t>(comb.memory);
                const bool lessThan = elementAddr < comb_begin;
                const bool greaterThan = (comb_begin - elementAddr) > RowWidth;
                if(lessThan || greaterThan){
                    continue;
                }
                const std::size_t index_into_comb = (elementAddr - comb_begin) / sizeof(T);
                if(!comb.occupancy[index_into_comb]){
                    throw std::logic_error("the hive did not construct this pointer");
                }
                comb.occupancy.set(index_into_comb, false);
                return;
            }

            throw std::out_of_range("trying to destroy an element not contained within the hive");
        }
        
        void AddRow() {
            combs.emplace_back();
        }
        void EraseRow(std::size_t combIndex) {
            combs.erase(combs.begin() + combIndex);
        }

        //add a begin and end for iteration
    };
}