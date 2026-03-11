#pragma once

#include "EightWinds/VulkanHeader.h"

namespace EWE{
	
    namespace ParamPack{
        struct Pipeline{
            VkPipeline pipe;
            VkPipelineLayout layout;
            VkPipelineBindPoint bindPoint;
        };
        struct VertexDraw{
            uint32_t vertexCount;
            uint32_t instanceCount;
            uint32_t firstVertex;
            uint32_t firstInstance;
        };
        struct IndexDraw{
            uint32_t indexCount;
            uint32_t instanceCount;
            uint32_t firstIndex;
            uint32_t vertexOffset;
            uint32_t firstInstance;
        };
        struct Label{
            const char* name;
            float red;
            float green;
            float blue;
        };
        struct Dispatch{
            uint32_t x;
            uint32_t y;
            uint32_t z;
        };
        struct DrawMeshTasks{
            uint16_t x;
            uint16_t y;
            uint16_t z;  
        };
        
        struct DrawIndirect{
            VkBuffer buffer;
            VkDeviceSize offset;
            uint32_t drawCount;
            uint32_t stride;
        };
        struct DrawIndirectCount {
            VkBuffer buffer;
            VkDeviceSize offset;
            VkBuffer countBuffer;
            VkDeviceSize countBufferOffset;
            uint32_t drawCount;
            uint32_t stride;
        };
        
        //same data
        using DrawIndexedIndirect = DrawIndirect;
        using DrawIndexedIndirectCount = DrawIndirectCount;
        using DrawMeshTasksIndirect = DrawIndirect;
        using DrawMeshTasksIndirectCount = DrawIndirectCount;

        struct DispatchIndirect{
            VkBuffer buffer;
            VkDeviceSize offset;
        };

        struct Viewport{
            VkViewport viewport;
        };
        struct Scissor{
            VkRect2D scissor;
        };
        struct ViewportCount{
            static constexpr uint32_t ArbitraryViewportCountLimit = 10;
            //lets set an arbitrary limit to 10
            VkViewport viewports[ArbitraryViewportCountLimit];
            uint32_t currentViewportCount = 0;
        };
        struct ScissorCount{
            static constexpr uint32_t ArbitraryScissorCountLimit = 10;
            VkRect2D scissors[ArbitraryScissorCountLimit];
            uint32_t currentScissorCount = 0;
        };

        struct ForLoop { //i assume this would need to be used with indirect commands
            int begin; 
            int comparison;
            int increment;
            //for (int i = begin; i < comparison; i += increment)
        };
    } //namespace ParamPack
} //namespace EWE