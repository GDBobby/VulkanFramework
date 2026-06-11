#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/LogicalDevice.h"
#include "EightWinds/Command/Instruction.h"

#include "EightWinds/Data/PerFlight.h"

#include "EightWinds/Data/HeapBlock.h"

#include "EightWinds/Command/ParamPool.h"

namespace EWE{

    struct CommandBuffer;

//struct DependencyHeader {
//    uint16_t buffer_count;
//    uint16_t image_count;
//};
namespace Command{
    void ExecuteParamPool(LogicalDevice& logicalDevice, CommandBuffer& cmdBuf, ParamPool const& pp, uint8_t frameIndex);
} //namespace Command 
} //namespace EWE