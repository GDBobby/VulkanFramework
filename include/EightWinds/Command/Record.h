#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/Command/Instruction.h"

#include "EightWinds/RenderGraph/GPUTask.h"

#include <array>
#include <string>

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

        //if it was compiled, i want an error when another command is added
        //unless i want duplicate command lists, that only have minor differentiations
        bool wasCompiled = false;

#if 1 //COMMAND_RECORD_NAMING
        std::string name;
#endif
    /*
        pool of params is 1 option
        that would be std::vector<uint8_t> or something

        other options - lambdas, use reference capture (keep lifetime in mind)
        lambdas are going to be extensively more expensive, but drastically easier (idk)
    */

        //this isnt going to help me setup bindless at all.
        //currently, it would need to be setup externally, and dependencies would need to be tracked externally.
        
        std::vector<CommandInstruction> records{};
        //i think i need GPUTask::Execute to just write directly to the param pool, so I don't need to worry about references/pointers
        //thats things like vertex count
        //param pool is going into GPUTask
        //std::vector<uint8_t> paramPool;

        GPUTask Compile() noexcept;

        //the push constant size needs the pipeline id.
        //optimization would need push constant size known at compile time (or optimization time, if theyre separate)
        //potentially, write optimized instructions to a file and read it back later
        void BindPipeline(Pipeline*);

        //i think i want a descirptor set that contains the details for buffers and images contained
        //im not sure how to do this yet
        //with some further inspection, i want to exclusively use device buffer address
        void BindDescriptor();

        void Push();

        //this shouldnt be used directly
        void BeginRender();
        void EndRender();

        //this needs to be expanded
        //potentially dont even allow this to be called explicitly?
        void Barrier();

        void BeginLabel() noexcept;
        void EndLabel() noexcept;

        void SetDynamicState(VkDynamicState dynState);

        void Draw();
        void DrawIndexed();
        void Dispatch();
    };

}//namespace EWE