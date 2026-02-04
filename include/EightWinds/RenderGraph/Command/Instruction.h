#pragma once

#include "EightWinds/VulkanHeader.h"

#include "EightWinds/RenderGraph/Command/ParamPacks.h"

#include <cstdint>

namespace EWE{

    struct CommandInstruction{
        enum class Type : uint8_t {
            BindPipeline,

            //descriptor needs to be expanded to handle images and buffers
            BindDescriptor,

            PushConstant, 

            BeginRender,
            EndRender,

            Draw,
            DrawIndexed,
            Dispatch,
            DrawMeshTasks,
            
            DrawIndirect,
            DrawIndexedIndirect,
            DispatchIndirect,
            DrawMeshTasksIndirect,
            
            DrawIndirectCount,
            DrawIndexedIndirectCount,
            DrawMeshTasksIndirectCount,

            //present is handled explicitly
            //Present,

            //PipelineBarrier,

            //dynamicstate
            DS_ViewportScissor,
            DS_ViewportScissorWithCount,

            BeginLabel,
            EndLabel,

            //control flow
            If,

            //im not sure if i want to do more advanced control flow yet
            LoopBegin, //using this as a for loop right now. POTENTIALLY use a while loop for conditionals, but idk

            Switch,
            Case,
            Default,

            //these 3 dont have any independent functionality. do i even need each control flow type to have it's own unique footer, or can I use one for all?
            EndIf,
            LoopEnd,
            SwitchEnd,

            //im only going to implement vp scissor for the moment
            DS_LineWidth,
            DS_DepthBias,
            DS_BlendConstants,
            DS_DepthBounds,
            DS_StencilCompareMask,
            DS_StencilWriteMask,
            DS_StencilReference,
            DS_CullMode,
            DS_FrontFace,
            DS_PrimitiveTopology,
            DS_DepthTestEnable,
            DS_DepthWriteEnable,
            DS_DepthCompareOp,
            DS_DepthBoundsTestEnable,
            DS_StencilTestEnable,
            DS_StencilOp,
            DS_RasterizerDiscardEnable,
            DS_DepthBiasEnable,
            DS_PrimitiveRestartEnable,
            //wtf is a stipple
            //DS_LineStipple,

        };

        Type type;
        std::size_t paramOffset;

        CommandInstruction(Type type, std::size_t offset)
            : type(type), paramOffset(offset) 
        {}

        static uint64_t GetParamSize(Type type) noexcept;
    };

} //namespace EWE