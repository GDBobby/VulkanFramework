#pragma once

#include "EightWinds/Data/PerFlight.h"

#include <array>

namespace EWE{
    //per flight cant be viewed by debug inspectors because they give 
    template<typename Resource>
    struct PerFlightDebugViewer{
        Resource& resource0;
        Resource& resource1;

        [[nodiscard]] explicit PerFlightDebugViewer(PerFlight<Resource>& source) noexcept
        : resource0{source[0]}, resource1{source[1]}
        {}
    };
}