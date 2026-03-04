#pragma once

#include <cstdint>
#include <functional>
#include <unordered_set>

#include <mutex>
#include <cstring> //memcmp
#include <ranges>

namespace EWE{
    struct Address{
        union{
            std::size_t address;
            void* pointer;
        };

        constexpr Address() : address{0} {}
        constexpr Address(std::size_t addr)
        : address{addr}
        {}
        template<typename T>
        constexpr Address(T* ptr)
        : pointer{reinterpret_cast<void*>(ptr)}
        {}

        bool operator==(Address const& other) const{
            //this avoids selecting either one from the union
            return memcmp(this, &other, sizeof(Address)) == 0;
        }
    };
} //namespace EWE

template <>
struct std::hash<EWE::Address> {
    std::size_t operator()(const EWE::Address& a) const noexcept {
        return std::hash<std::size_t>{}(a.address);
    }
};

namespace EWE{

    //this is buffers, command pools, and images
    template<typename T>
    struct ResourceTracker{
        std::mutex mut;
        std::unordered_set<Address> resources;

        void Add(T* res){
            mut.lock();
            resources.insert(res);
            mut.unlock();
        }
        void Remove(T* res){
            mut.lock();
            resources.erase(res);
            mut.unlock();
        }

        struct Iterator {
            using iterator_category = std::forward_iterator_tag;
            using value_type = T*;
            using difference_type = std::ptrdiff_t;
            using pointer = T**;
            using reference = T*&;

            std::unordered_set<Address>::iterator it;

            T* operator*() const { return static_cast<T*>(it->pointer); }
            Iterator& operator++() { ++it; return *this; }
            bool operator!=(const Iterator& other) const { return it != other.it; }
        };

        auto begin() { return Iterator{ resources.begin() }; }
        auto end()   { return Iterator{ resources.end() };   }
    };
    
} //namespace EWE