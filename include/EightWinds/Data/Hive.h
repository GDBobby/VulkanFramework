#pragma once

#include "EightWinds/Preprocessor.h"

#include "EightWinds/Data/StackBlock.h"
#include "EightWinds/Data/HeapBlock.h"

#include <cstdint>
#include <type_traits>
#include <concepts>
#include <memory>

#include <vector>
#include <bitset>

namespace EWE{
    //based on a very cursory glance at std::hive (P0447R21)
    template<typename T, std::size_t RowWidth = 16>
    struct Hive{

        struct Comb{
            StackBlock<T, RowWidth> memory;
            std::bitset<RowWidth> occupancy;

            [[nodiscard]] Comb()
            : memory{},
                occupancy{}
            {}
            ~Comb(){
#if 0//EWE_DEBUG_BOOL //idk if i want this, maybe just destruct
                if(occupancy[rowIndex].any()){
                    throw std::invalid_argument("freeing objects that have not been destroyed");
                }
#else
                for(std::size_t i = 0; i < RowWidth; i++){
                    if(occupancy[i]){
                        memory.DestroyAt(i);
                    }
                }
#endif
            }

            Comb(Comb const& copySrc) = delete;
            Comb& operator=(Comb const& copySrc) = delete;
            Comb& operator=(Comb&& moveSrc) noexcept = delete;

            Comb(Comb&& moveSrc) noexcept = delete;
            /*
            : memory{moveSrc.memory}, 
                occupancy{moveSrc.occupancy}
            {   
                moveSrc.memory = nullptr;
            }
            */
        };
        std::vector<Comb*> combs;

        [[nodiscard]] explicit Hive()
            : combs {}
        {
            AddRow();
        }

        T* GetAConstructionPointer(){
            for(auto& comb : combs){
                if(!comb->occupancy.all()){
                    for(std::size_t i = 0; i < RowWidth; i++){
                        if(!comb->occupancy[i]){
                            comb->occupancy.set(i, true);
                            return reinterpret_cast<T*>(comb->memory.GetMemory() + (sizeof(T) * i));
                        }
                    }
                }
            }
            AddRow();
            auto& combBack = combs.back();
            combBack->occupancy.set(0, true);
            return combBack->memory.GetMemory();
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
                const std::size_t comb_begin = reinterpret_cast<std::size_t>(comb->memory.GetMemory());
                const bool lessThan = elementAddr < comb_begin;
                const bool greaterThan = (comb_begin - elementAddr) > RowWidth;
                if(lessThan || greaterThan){
                    continue;
                }
                const std::size_t index_into_comb = (elementAddr - comb_begin) / sizeof(T);
                if(!comb->occupancy[index_into_comb]){
                    throw std::logic_error("the hive did not construct this pointer");
                }
                comb->occupancy.set(index_into_comb, false);
                return;
            }

            throw std::out_of_range("trying to destroy an element not contained within the hive");
        }
        
        void AddRow() {
            combs.push_back(new Comb());
        }
        void EraseRow(std::size_t combIndex) {
            delete combs[combIndex];
            combs.erase(combs.begin() + combIndex);
        }

        //add a begin and end for iteration
        struct iterator {
            Hive* hive = nullptr;
            std::size_t combIndex = 0;
            std::size_t slotIndex = 0;

            iterator(Hive* hive, std::size_t combIndex, std::size_t slotIndex)
                : hive(hive), combIndex(combIndex), slotIndex(slotIndex) {
                advance_to_valid();
            }

            void advance_to_valid() {
                while (combIndex < hive->combs.size()) {
                    auto& comb = *hive->combs[combIndex];
                    while (slotIndex < RowWidth) {
                        if (comb.occupancy.test(slotIndex))
                            return;
                        ++slotIndex;
                    }
                    slotIndex = 0;
                    ++combIndex;
                }
            }

            T& operator*() const {
                return hive->combs[combIndex]->memory[slotIndex];
            }

            T* operator->() const {
                return &(**this);
            }

            iterator& operator++() {
                ++slotIndex;
                advance_to_valid();
                return *this;
            }

            bool operator==(iterator const& other) const {
                return hive == other.hive &&
                    combIndex == other.combIndex &&
                    slotIndex == other.slotIndex;
            }

            bool operator!=(iterator const& other) const {
                return !(*this == other);
            }
        };

        struct const_iterator {
            const Hive* hive = nullptr;
            std::size_t combIndex = 0;
            std::size_t slotIndex = 0;

            const_iterator(const Hive* hive, std::size_t combIndex, std::size_t slotIndex)
                : hive(hive), combIndex(combIndex), slotIndex(slotIndex) {
                advance_to_valid();
            }

            void advance_to_valid() {
                while (combIndex < hive->combs.size()) {
                    const auto& comb = *hive->combs[combIndex];
                    while (slotIndex < RowWidth) {
                        if (comb.occupancy.test(slotIndex))
                            return;
                        ++slotIndex;
                    }
                    slotIndex = 0;
                    ++combIndex;
                }
            }

            const T& operator*() const {
                return hive->combs[combIndex]->memory[slotIndex];
            }

            const_iterator& operator++() {
                ++slotIndex;
                advance_to_valid();
                return *this;
            }

            bool operator==(const_iterator const& other) const {
                return hive == other.hive &&
                    combIndex == other.combIndex &&
                    slotIndex == other.slotIndex;
            }

            bool operator!=(const_iterator const& other) const {
                return !(*this == other);
            }
        };

        iterator begin() {
            return iterator(this, 0, 0);
        }

        iterator end() {
            return iterator(this, combs.size(), 0);
        }

        const_iterator begin() const {
            return const_iterator(this, 0, 0);
        }

        const_iterator end() const {
            return const_iterator(this, combs.size(), 0);
        }

        const_iterator cbegin() const {
            return begin();
        }

        const_iterator cend() const {
            return end();
        }
    };
}