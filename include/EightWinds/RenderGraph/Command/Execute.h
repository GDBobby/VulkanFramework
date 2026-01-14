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
    
    struct CommandExecutor{
        LogicalDevice& logicalDevice;

        [[nodiscard]] explicit CommandExecutor(LogicalDevice& logicalDevice) noexcept;
        CommandExecutor(CommandExecutor const& copySrc) = delete;
        CommandExecutor(CommandExecutor&& moveSrc) = delete;
        CommandExecutor& operator=(CommandExecutor const& copySrc) = delete;
        CommandExecutor& operator=(CommandExecutor&& moveSrc) = delete;

        //i can use templates to make the parampool type aware
        PerFlight<std::vector<uint8_t>> paramPool;
        //std::vector<uint8_t> barrierPool;
        std::vector<CommandInstruction> instructions{};

        void Execute(CommandBuffer& cmdBuf, uint8_t frameIndex) const noexcept;
        void Debug() const noexcept;
    };
}