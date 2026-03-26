#pragma once

#include "EightWinds/VulkanHeader.h"

#include "EightWinds/RenderGraph/Command/Instruction.h"
#include "Instruction.h"

namespace EWE{

    template<Instruction::Type>
    struct ParamPack;

    template<> struct ParamPack<Instruction::BindPipeline>{
        VkPipeline pipe;
        VkPipelineLayout layout;
        VkPipelineBindPoint bindPoint;
    };
    template<> struct ParamPack<Instruction::Draw>{
        uint32_t vertexCount;
        uint32_t instanceCount;
        uint32_t firstVertex;
        uint32_t firstInstance;
    };
    template<> struct ParamPack<Instruction::DrawIndexed>{
        uint32_t indexCount;
        uint32_t instanceCount;
        uint32_t firstIndex;
        uint32_t vertexOffset;
        uint32_t firstInstance;
    };
    template<> struct ParamPack<Instruction::BeginLabel>{
        const char* name;
        float red;
        float green;
        float blue;
    };
    template<> struct ParamPack<Instruction::Dispatch>{
        uint32_t x;
        uint32_t y;
        uint32_t z;
    };
    template<> struct ParamPack<Instruction::DrawMeshTasks>{
        uint16_t x;
        uint16_t y;
        uint16_t z;  
    };
    template<> struct ParamPack<Instruction::DrawIndirect>{
        VkBuffer buffer;
        VkDeviceSize offset;
        uint32_t drawCount;
        uint32_t stride;
    };
    template<> struct ParamPack<Instruction::DrawIndirectCount>{
        VkBuffer buffer;
        VkDeviceSize offset;
        VkBuffer countBuffer;
        VkDeviceSize countBufferOffset;
        uint32_t drawCount;
        uint32_t stride;
    };
    //same data
    template<> struct ParamPack<Instruction::DrawIndexedIndirect> 
        : public ParamPack<Instruction::DrawIndirect>
    {};
    template<> struct ParamPack<Instruction::DrawIndexedIndirectCount> 
        : public ParamPack<Instruction::DrawIndirectCount>
    {};
    template<> struct ParamPack<Instruction::DrawMeshTasksIndirect> 
        : public ParamPack<Instruction::DrawIndirect>
    {};
    template<> struct ParamPack<Instruction::DrawMeshTasksIndirectCount> 
        : public ParamPack<Instruction::DrawIndirect>
    {};

    template<> struct ParamPack<Instruction::DispatchIndirect>{
        VkBuffer buffer;
        VkDeviceSize offset;
    };
    template<> struct ParamPack<Instruction::DS_Viewport>{
        VkViewport viewport;
    };
    template<> struct ParamPack<Instruction::DS_Scissor>{
        VkRect2D scissor;
    };
    template<> struct ParamPack<Instruction::DS_ViewportCount>{
        static constexpr uint32_t ArbitraryViewportCountLimit = 10;
        //lets set an arbitrary limit to 10
        VkViewport viewports[ArbitraryViewportCountLimit];
        uint32_t currentViewportCount = 0;
    };
    template<> struct ParamPack<Instruction::DS_ScissorCount>{
        static constexpr uint32_t ArbitraryScissorCountLimit = 10;
        VkRect2D scissors[ArbitraryScissorCountLimit];
        uint32_t currentScissorCount = 0;
    };
    template<> struct ParamPack<Instruction::LoopBegin>{ //i assume this would need to be used with indirect commands
        int begin; 
        int comparison;
        int increment;
        //for (int i = begin; i < comparison; i += increment)
    };
    //template<> struct ParamPack<>{};
	
} //namespace EWE