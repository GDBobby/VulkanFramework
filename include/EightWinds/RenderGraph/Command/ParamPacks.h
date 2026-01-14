#pragma once

#include "EightWinds/VulkanHeader.h"

namespace EWE{
	
    struct PipelineParamPack{
        VkPipeline pipe;
        VkPipelineLayout layout;
        VkPipelineBindPoint bindPoint;
    };
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
    struct DispatchParamPack{
        uint32_t x;
        uint32_t y;
        uint32_t z;
    };
    struct DrawMeshTasksParamPack{
        uint8_t x;
        uint8_t y;
        uint8_t z;  
    };
    
    struct DrawIndirectParamPack{
        VkBuffer buffer;
        VkDeviceSize offset;
        uint32_t drawCount;
        uint32_t stride;
    };
    struct DrawIndirectCountParamPack{
        VkBuffer buffer;
        VkDeviceSize offset;
        VkBuffer countBuffer;
        VkDeviceSize countBufferOffset;
        uint32_t drawCount;
        uint32_t stride;
    };
    struct DispatchIndirectParamPack{
        VkBuffer buffer;
        VkDeviceSize offset;
    };
    
    struct ViewportScissorParamPack{
        VkViewport viewport;
        VkRect2D scissor;
    };
    struct ViewportScissorWithCountParamPack{
        static constexpr uint32_t ArbitraryViewportCountLimit = 10;
        static constexpr uint32_t ArbitraryScissorCountLimit = 10;
        //lets set an arbitrary limit to 10
        VkViewport viewports[ArbitraryViewportCountLimit];
        VkRect2D scissors[ArbitraryScissorCountLimit];
        uint32_t currentViewportCount = 0;
        uint32_t currentScissorCount = 0;
    };
}