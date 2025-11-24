#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/CommandInstruction.h"
#include <vector>

namespace EWE{
    
    struct CommandExecutor{
        //i can use templates to make the parampool type aware
        std::vector<uint8_t> paramPool;
        std::vector<CommandInstruction> instructions{};

        void Execute(CommandBuffer& cmdBuf) const noexcept;
    };
}