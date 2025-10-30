#pragma once

#include "EightWinds/LogicalDevice.h"

namespace EWE{
    struct CommandPool{
        LogicalDevice& logicalDevice;
        Queue& queue;

        //release bits?
    };
}