#pragma once

#include "EightWinds/VulkanHeader.h"

#include "EightWinds/Command/InstructionType.h"
#include "EightWinds/GlobalPushConstant.h"

namespace EWE{

    template<Inst::Type IType>
    struct ParamPack;


    template<> struct ParamPack<Inst::Push> {
        uint32_t size; 
        uint8_t buffer_count; //cpu sided, the shader already knows
        uint8_t texture_count; //cpu sided, the shader already knows
        //assert(ele_count * ele_size == size);

        static constexpr std::size_t data_size = 128 / sizeof(std::byte);
        std::byte data[data_size];

        VkDeviceAddress& GetDeviceAddress(uint8_t index){
#if EWE_DEBUG_BOOL
            VkDeviceAddress* temp_array = reinterpret_cast<VkDeviceAddress*>(data);
            auto& ret = temp_array[index];
            return ret;
#else
            return reinterpret_cast<VkDeviceAddress*>(data)[index];
#endif
        }
        TextureIndex& GetTextureIndex(uint8_t index){
            auto* starting_addr = reinterpret_cast<VkDeviceAddress*>(data) + buffer_count;
            return reinterpret_cast<TextureIndex*>(starting_addr)[index];
        }

        std::size_t Size() const{
            return buffer_count * sizeof(VkDeviceAddress)
                + texture_count * sizeof(TextureIndex);
        }
    };
    template<> struct ParamPack<Inst::BeginRender> : public VkRenderingInfo{
        using VkRenderingInfo::operator=;
    }; //i need to figure something out here

    template<> struct ParamPack<Inst::BindPipeline>{
        VkPipeline pipe;
        VkPipelineLayout layout;
        VkPipelineBindPoint bindPoint;
    };
    template<> struct ParamPack<Inst::Draw>{
        uint32_t vertexCount;
        uint32_t instanceCount;
        uint32_t firstVertex;
        uint32_t firstInstance;
    };
    struct IndexBuffer{
        VkBuffer buffer; //pointer
        VkDeviceSize offset;
        VkIndexType indexType;

        bool operator==(IndexBuffer& other) const{
            return buffer == other.buffer 
            && offset == other.offset 
            && indexType == other.indexType;
        }
    };

    template<> struct ParamPack<Inst::DrawIndexed>{
        //is it a mistake to tightly bind these 2?
        //i dont see a common use case where i want to do multiple draw calls per index buffer without instancing
        IndexBuffer indexBuffer;

        uint32_t indexCount;
        uint32_t instanceCount;
        uint32_t firstIndex;
        uint32_t vertexOffset;
        uint32_t firstInstance;
    };
    template<> struct ParamPack<Inst::BeginLabel>{
        const char* name;
        float red;
        float green;
        float blue;
    };
    template<> struct ParamPack<Inst::Dispatch>{
        uint32_t x;
        uint32_t y;
        uint32_t z;
    };
    template<> struct ParamPack<Inst::DrawMeshTasks>{
        uint16_t x;
        uint16_t y;
        uint16_t z;  
    };
    template<> struct ParamPack<Inst::DrawIndirect>{
        VkBuffer buffer;
        VkDeviceSize offset;
        uint32_t drawCount;
        uint32_t stride;
    };
    template<> struct ParamPack<Inst::DrawIndirectCount>{
        VkBuffer buffer;
        VkDeviceSize offset;
        VkBuffer countBuffer;
        VkDeviceSize countBufferOffset;
        uint32_t drawCount;
        uint32_t stride;
    };
    //same data
    template<> struct ParamPack<Inst::DrawIndexedIndirect> 
        : public ParamPack<Inst::DrawIndirect>
    {};
    template<> struct ParamPack<Inst::DrawIndexedIndirectCount> 
        : public ParamPack<Inst::DrawIndirectCount>
    {};
    template<> struct ParamPack<Inst::DrawMeshTasksIndirect> 
        : public ParamPack<Inst::DrawIndirect>
    {};
    template<> struct ParamPack<Inst::DrawMeshTasksIndirectCount> 
        : public ParamPack<Inst::DrawIndirectCount>
    {};

    template<> struct ParamPack<Inst::DispatchIndirect>{
        VkBuffer buffer;
        VkDeviceSize offset;
    };
    template<> struct ParamPack<Inst::DS_Viewport>{
        VkViewport viewport;
    };
    template<> struct ParamPack<Inst::DS_Scissor>{
        VkRect2D scissor;
    };
    template<> struct ParamPack<Inst::DS_ViewportCount>{
        static constexpr uint32_t ArbitraryViewportCountLimit = 10;
        //lets set an arbitrary limit to 10
        VkViewport viewports[ArbitraryViewportCountLimit];
        uint32_t currentViewportCount = 0;
    };
    template<> struct ParamPack<Inst::DS_ScissorCount>{
        static constexpr uint32_t ArbitraryScissorCountLimit = 10;
        VkRect2D scissors[ArbitraryScissorCountLimit];
        uint32_t currentScissorCount = 0;
    };

    template<> struct ParamPack<Inst::If>{
        bool enabled;

        operator bool() const{
            return enabled;
        }
    };

    template<> struct ParamPack<Inst::LoopBegin>{ //i assume this would need to be used with indirect commands
        int begin; 
        int comparison;
        int increment;
        //for (int i = begin; i < comparison; i += increment)
    };
    //template<> struct ParamPack<>{};

    namespace Command{
        struct ParamPool;
    }
    template<> struct ParamPack<Inst::Ext_Pool>{
        Command::ParamPool* pool;
    };
	
} //namespace EWE