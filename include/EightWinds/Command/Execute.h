#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/LogicalDevice.h"
#include "EightWinds/Command/Instruction.h"

#include "EightWinds/Data/PerFlight.h"

#include "EightWinds/Data/HeapBlock.h"

namespace EWE{

    struct CommandBuffer;

//struct DependencyHeader {
//    uint16_t buffer_count;
//    uint16_t image_count;
//};
namespace Command{
    struct Record;

    struct Executor{
        LogicalDevice& logicalDevice;
        Record& record;

        [[nodiscard]] explicit Executor(LogicalDevice& logicalDevice, Record& record) noexcept;
        Executor(Executor const& copySrc) = delete;
        Executor(Executor&& moveSrc) = delete;
        Executor& operator=(Executor const& copySrc) = delete;
        Executor& operator=(Executor&& moveSrc) = delete;

        //i can use templates to make the parampool type aware
        PerFlight<HeapBlock<uint8_t>> paramPool;

        void Execute(CommandBuffer& cmdBuf, uint8_t frameIndex) const noexcept;
        void Debug() const noexcept;
    };
} //namespace Command 
} //namespace EWE