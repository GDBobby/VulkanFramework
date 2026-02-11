#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/LogicalDevice.h"
#include "EightWinds/RenderGraph/Command/Instruction.h"

#include "EightWinds/Data/PerFlight.h"

#include <vector>

namespace EWE{

    struct CommandBuffer;

    //struct DependencyHeader {
    //    uint16_t buffer_count;
    //    uint16_t image_count;
    //};
    namespace Command{
        struct Executor{
            LogicalDevice& logicalDevice;

            [[nodiscard]] explicit Executor(LogicalDevice& logicalDevice) noexcept;
            Executor(Executor const& copySrc) = delete;
            Executor(Executor&& moveSrc) = delete;
            Executor& operator=(Executor const& copySrc) = delete;
            Executor& operator=(Executor&& moveSrc) = delete;

            //i can use templates to make the parampool type aware
            PerFlight<std::vector<uint8_t>> paramPool;
            //std::vector<uint8_t> barrierPool;
            std::vector<Instruction> instructions{};

            void Execute(CommandBuffer& cmdBuf, uint8_t frameIndex) const noexcept;
            void Debug() const noexcept;
        };
    } //namespace Command 
} //namespace EWE