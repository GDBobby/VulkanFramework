#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/CommandInstruction.h"

#include "EightWinds/RenderGraph/GPUTask.h"

#include <array>

namespace EWE{
    //forward declared just to save a bit of compile time
    //the implementation doesnt matter here
    struct Pipeline;
    struct PipelineBarrier;
    struct CommandBuffer;

    struct CommandRecord{
        CommandRecord() = default;
        CommandRecord(CommandRecord const&) = delete;
        CommandRecord& operator=(CommandRecord const&) = delete;
        CommandRecord(CommandRecord&&) = delete;
        CommandRecord& operator=(CommandRecord&&) = delete;

#if 1 //COMMAND_RECORD_NAMING
        std::string name;
#endif
    /*
        pool of params is 1 option
        that would be std::vector<uint8_t> or something

        other options - lambdas, use reference capture (keep lifetime in mind)
        lambdas are going to be extensively more expensive, but potentially drastically easier (idk)
    */

        //this isnt going to help me setup bindless at all.
        //currently, it would need to be setup externally, and dependencies would need to be tracked externally.
        
        std::vector<CommandInstruction> records{};
        //i think i need GPUTask::Execute to just write directly to the param pool, so I don't need to worry about references/pointers
        //thats things like vertex count if it becomes variable
        //param pool is going into GPUTask
        //std::vector<uint8_t> paramPool;

        GPUTask Compile() noexcept;


        void BindPipeline();

        //i think i want a descirptor set that contains the details for buffers and images contained
        //im not sure how to do this yet
        void BindDescriptor();

        //if i force Progammable Vertex Pulling these won't be used
        void BindVertexBuffer();
        void BindIndexBuffer();

        void Push(void* push, std::size_t pushSize);

        //this shouldnt be used directly
        void BeginRender();
        void EndRender();

        //this needs to be expanded
        //potentially dont even allow this to be called explicitly?
        void Barrier();

        void BeginLabel(const char* name, float red, float green, float blue) noexcept;
        void EndLabel() noexcept;

        void SetDynamicState(VkDynamicState dynState);

        void Draw();
        void DrawIndexed();
        void Dispatch();
    };

}//namespace EWE