#pragma once

//i dont need vulkan, i just need max_frames_in_flight on a wider scale than here
#include "EightWinds/VulkanHeader.h"

#include "EightWinds/Data/ForwardArgConstructionHelper.h"


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
        alignas(Resource) uint8_t buffer[max_frames_in_flight * sizeof(Resource)];
#if EWE_DEBUG_BOOL
        Resource* firstPtr_debug;
        Resource* secondPtr_debug; 
#endif

        constexpr Resource* resources() noexcept {
            return reinterpret_cast<Resource*>(buffer);
        }
        constexpr Resource const* resources() const noexcept {
            return reinterpret_cast<const Resource*>(buffer);
        }
            
        constexpr Resource& operator[](size_t i) noexcept { return resources()[i]; }
        constexpr Resource const& operator[](size_t i) const noexcept { return resources()[i]; }

        template <typename... Rs>
        requires (sizeof...(Rs) == max_frames_in_flight && (std::constructible_from<Resource, Rs&&> && ...))
        [[nodiscard]] explicit PerFlight(Rs&&... rs) {
            size_t i = 0;
            (std::construct_at(&resources()[i++], std::forward<Rs>(rs)), ...);
#if EWE_DEBUG_BOOL
            firstPtr_debug = &resources()[0];
            secondPtr_debug = &resources()[1];
#endif
        }
        //this function constructs every resource in the buffer with the same arguments
        template <typename... Args>
        requires std::constructible_from<Resource, Args...>
        [[nodiscard]] PerFlight(Args&&... args) {
            for (size_t i = 0; i < max_frames_in_flight; ++i) {
                std::construct_at(&resources()[i], std::forward<Args>(args)...);
            }
#if EWE_DEBUG_BOOL
            firstPtr_debug = &resources()[0];
            secondPtr_debug = &resources()[1];
#endif
        }


        //this constructs Resource with a reference of type Other, per object in the buffer
        template<typename Other>
        requires(std::is_constructible_v<Resource, Other&>)
        [[nodiscard]] explicit PerFlight(PerFlight<Other>& other) {

            for (std::size_t i = 0; i < max_frames_in_flight; ++i) {
                std::construct_at(&resources()[i], other.resources()[i]);
            }
#if EWE_DEBUG_BOOL
            firstPtr_debug = &resources()[0];
            secondPtr_debug = &resources()[1];
#endif
        }

        template<std::size_t ArgsForFirstObject, typename... Args>
            requires (ArgsForFirstObject <= sizeof...(Args)) && (max_frames_in_flight == 2) //max_fif == 3 will need to be addressed separately
        PerFlight(ArgumentPack_ConstructionHelper<ArgsForFirstObject> helper, Args&&... args) {

            constexpr std::size_t remaining_args = sizeof...(Args) - ArgsForFirstObject;

            ConstructFrom_ForwardedArgumentPackSlice<0, ArgsForFirstObject>(resources(), std::forward<Args>(args)...);
            ConstructFrom_ForwardedArgumentPackSlice<ArgsForFirstObject, remaining_args>(resources() + 1, std::forward<Args>(args)...);
#if EWE_DEBUG_BOOL
            firstPtr_debug = &resources()[0];
            secondPtr_debug = &resources()[1];
#endif
        }

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