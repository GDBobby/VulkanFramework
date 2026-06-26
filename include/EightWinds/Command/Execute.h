#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/LogicalDevice.h"
#include "EightWinds/Command/Instruction.h"

#include "EightWinds/Data/PerFlight.h"

#include "EightWinds/Data/HeapBlock.h"

#include "EightWinds/Command/ParamPool.h"

namespace EWE{

    struct CommandBuffer;

namespace Command{
#if EWE_DEBUG_BOOL
namespace Exec{
    struct ExecContext {
        LogicalDevice& device;
        ParamPool const& pp; //its important that this is a view and not a copy or move, either of which would invalidate pointers

        CommandBuffer& cmdBuf;

        ParamPack<Inst::BindPipeline> boundPipeline{};

        std::size_t iterator = 0;

        uint8_t frame;

        Address address;

        template <Inst::Type IType>
        requires(Inst::GetParamSize(IType) > 0)
        auto& CastAndIncrement(){
            auto& ret = address.CastToRef<ParamPack<IType>>();
            address.address += sizeof(ParamPack<IType>);
            return ret;
        }

        void Iterate();
    };
} //namespace Exec
#endif

    void ExecuteParamPool(LogicalDevice& logicalDevice, CommandBuffer& cmdBuf, ParamPool const& pp, uint8_t frameIndex);
} //namespace Command 
} //namespace EWE