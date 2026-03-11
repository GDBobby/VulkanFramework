#pragma once

#include "EightWinds/Data/PerFlight.h"

#if EWE_DEBUG_BOOL
#include <cassert>
#endif
#include <cstdint>


namespace EWE{
    template<typename T>
    struct InstructionPointer{
    private:
        PerFlight<T*> data;
        //for use with param pool if true, 
        //if not, the user defines the construction and lifetime of the pointed data
        //if internal is not true, the pointer will not be changed when calling Record::Undefer
        bool internal = true;
        bool adjusted = false;
    public:

        InstructionPointer() : data{ reinterpret_cast<T*>(UINT64_MAX) } {}
        InstructionPointer(std::size_t offset)
        : data{reinterpret_cast<T*>(offset) }
        {}

        constexpr T& GetRef(uint8_t frameIndex) {
#if EWE_DEBUG_BOOL
            assert(frameIndex < max_frames_in_flight);
            assert(adjusted);
#endif
            return *data[frameIndex];
        }
    };

    struct InstructionPointerAdjuster {
        PerFlight<std::size_t> data;
        bool adjusted;
        bool internal; 
    };
}