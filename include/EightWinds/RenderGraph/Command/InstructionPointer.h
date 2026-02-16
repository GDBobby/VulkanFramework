#pragma once

#include "EightWinds/Data/PerFlight.h"

#if EWE_DEBUG_BOOL
#include <cassert>
#endif


namespace EWE{
    template<typename T>
    struct InstructionPointer{
    private:
        PerFlight<T*> data;
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
    };
}