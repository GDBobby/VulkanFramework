#pragma once

#include "EightWinds/Data/PerFlight.h"

#include <cstdint>


namespace EWE{
    template<typename T>
    struct InstructionPointer{
    private:
        PerFlight<T*> data;
        //for use with param pool if true, 
        //if not, the user defines the construction and lifetime of the pointed data
        //if internal is not true, the pointer will not be changed when calling Record::Undefer
        bool adjusted = false;
    public:

        InstructionPointer() : data{ reinterpret_cast<T*>(UINT64_MAX) } {}
        InstructionPointer(std::size_t offset)
        : data{reinterpret_cast<T*>(offset) }
        {}

        constexpr T& GetRef(uint8_t frameIndex) {
            //EWE_ASSERT(adjusted);
            return *data[frameIndex];
        }
    };

    struct InstructionPointerAdjuster {
        PerFlight<std::size_t> data;
        bool adjusted = false;

        template<typename T>
        InstructionPointer<T>* CastTo() noexcept{
            return reinterpret_cast<InstructionPointer<T>*>(this);
        }
    };
}