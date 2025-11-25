#pragma once

#include "EightWinds/VulkanHeader.h"

#include <cstdint>

namespace EWE{
    struct VertexDrawParamPack{
        uint32_t vertexCount;
        uint32_t instanceCount;
        uint32_t firstVertex;
        uint32_t firstInstance;
    };
    struct IndexDrawParamPack{
        uint32_t indexCount;
        uint32_t instanceCount;
        uint32_t firstIndex;
        uint32_t vertexOffset;
        uint32_t firstInstance;
    };
    struct LabelParamPack{
        const char* name;
        float red;
        float green;
        float blue;
    };

    struct CommandInstruction{
        enum class Type : uint8_t {
            BindPipeline,

            //descriptor needs to be expanded to handle images and buffers
            BindDescriptor,

            PushConstant,
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

            //im not sure if i want to do more advanced control flow yet
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
            : type(type), paramOffset(offset) 
        {}

        static uint64_t GetParamSize(Type type) noexcept;
    };

} //namespace EWE