#pragma once

//i dont need vulkan, i just need max_frames_in_flight on a wider scale than here
#include "EightWinds/VulkanHeader.h"

#include <array>


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

        template <typename... Rs>
        requires (sizeof...(Rs) == max_frames_in_flight && (std::constructible_from<Resource, Rs&&> && ...))
        [[nodiscard]] explicit PerFlight(Rs&&... rs) {
            size_t i = 0;
            (std::construct_at(&resources()[i++], std::forward<Rs>(rs)), ...);
        }
        //this function constructs every resource in the buffer with the same arguments
        template <typename... Args>
        requires std::constructible_from<Resource, Args...>
        [[nodiscard]] PerFlight(Args&&... args) {
            for (size_t i = 0; i < max_frames_in_flight; ++i) {
                std::construct_at(&resources()[i], std::forward<Args>(args)...);
            }
        }


        //this constructs Resource with a reference of type Other, per object in the buffer
        template<typename Other>
        requires(std::is_constructible_v<Resource, Other&>)
        [[nodiscard]] explicit PerFlight(PerFlight<Other>& other) {

            for (std::size_t i = 0; i < max_frames_in_flight; ++i) {
                std::construct_at(&resources()[i], other.resources()[i]);
            }
        }

        //Factory is a function that will construct Resource at the given address
        template <std::invocable<Resource*> Factory>
        PerFlight(Factory&& factory) {
            for (size_t i = 0; i < max_frames_in_flight; ++i) {
                std::invoke(factory, &resources()[i]);
            }
        }

        /*
        i'd like another function that takes full argument lists to construct each Resource, like so
        it might be a tuple thing

        template <typename... Args[max_frames_in_flight]>
        requires std::constructible_from<Resource, Args...>
        PerFlight(std::array<Args&&..., max_frames_in_flight> argList) {
        for(uint8_t i = 0; i < max_frames_in_flight; i++){
            std::construct_at(&resources()[i], argList[i]);
        }
        */

        ~PerFlight() {
            for (size_t i = 0; i < max_frames_in_flight; ++i) {
                std::destroy_at(&resources()[i]);
            }
        }

        Resource* begin() noexcept { return resources(); }
        Resource* end()   noexcept { return resources() + max_frames_in_flight; }

        const Resource* begin() const noexcept { return resources(); }
        const Resource* end()   const noexcept { return resources() + max_frames_in_flight; }

        const Resource* cbegin() const noexcept { return resources(); }
        const Resource* cend()   const noexcept { return resources() + max_frames_in_flight; }
    };
}