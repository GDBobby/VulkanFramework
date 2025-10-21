#pragma once

#include "Pin.h"

#include "LAB/Vector.h"

#include <cstdint>

namespace EWE{
    struct Link{
        Pin first;
        Pin second;

        lab::vec3 color;

        //bezier
    };
}