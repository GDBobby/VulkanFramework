#pragma once
#include <cstdint>
#include <concepts>
#include <memory>
#include <utility>

//typically, your average app will do 2 frames in flight?
//and sometimes they'll give a toggle for 3 frames in flight

//giving a toggle for 3 frames in flight gives up a tiny bit of performance. i'll have to check the frame count during runtime.
//if its literally just ONE check per frame, its essentially free.

//that difference will essentially live here. if its static constexpr, it's decided at compile time.
//if its not constexpr, it's decided at runtime

//if i make it template specialized, it'll DESTROY compile times. i'll have to compile the buffers and images with specialized templates
//and whatever owns those resources will have to be header only

namespace EWE{
    static constexpr uint8_t max_frames_in_flight = 2;

    template<typename Resource>
    struct PerFlight{
        alignas(Resource) unsigned char buffer[max_frames_in_flight * sizeof(Resource)];

        Resource* resources() noexcept {
            return reinterpret_cast<Resource*>(buffer);
        }
        Resource const* resources() const noexcept {
            return reinterpret_cast<const Resource*>(buffer);
        }
            
        Resource& operator[](size_t i) noexcept { return resources()[i]; }
        Resource const& operator[](size_t i) const noexcept { return resources()[i]; }

        template <typename... Args>
        requires std::constructible_from<Resource, Args...>
        PerFlight(Args&&... args) {
            for (size_t i = 0; i < max_frames_in_flight; ++i) {
                std::construct_at(&resources()[i], std::forward<Args>(args)...);
            }
        }

        ~PerFlight() {
            for (size_t i = 0; i < max_frames_in_flight; ++i) {
                std::destroy_at(&resources()[i]);
            }
        }
    };
}