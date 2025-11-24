#pragma once

#include "EightWinds/VulkanHeader.h"

#include <cstdint>

namespace EWE{
    struct CommandInstruction{
        enum class Type : uint8_t {
            BindPipeline,

            //descriptor needs to be expanded to handle images and buffers
            BindDescriptor,

            //id like to cut these two, with PVP
            BindVertexBuffer,
            BindIndexBuffer,

            PushConstants,
            SetDynamicState,

            BeginRender,
            EndRender,

            Draw,
            DrawIndexed,
            Dispatch,

            PipelineBarrier,

            BeginLabel,
            EndLabel,

            //control flow
            If,

            LoopBegin,

            Switch,
            Case,
            Default,

            //these 3 dont have any independent functionality
            EndIf,
            LoopEnd,
            SwitchEnd
        };

        Type type;
        std::size_t paramOffset;

        
        CommandInstruction(Type type, std::size_t offset)
            : type(type), paramOffset(offset), paramSize(GetParamSize(type)) {}

        CommandInstruction(Type type, std::size_t offset, std::size_t size)
            : type(type), paramOffset(offset), paramSize(size) {}

        //i could play it dangerous and not track this
        //let the pipeline control the size maybe?
        //the only size that isn't known is push constants
        std::size_t paramSize;

        static uint64_t GetParamSize(Type type) noexcept;
    };

} //namespace EWE