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
    struct GlobalPushConstant;
    struct RenderInfo;
    struct LogicalDevice;

    template<typename T>
    struct DeferredReference{
        T* data;

        DeferredReference(void* offset)
        : data{reinterpret_cast<T*>(offset)}
        {}
    };

    struct CommandRecord{
        CommandRecord() = default;
        CommandRecord(CommandRecord const&) = delete;
        CommandRecord& operator=(CommandRecord const&) = delete;
        CommandRecord(CommandRecord&&) = delete;
        CommandRecord& operator=(CommandRecord&&) = delete;

        //if i want compile time optimization, i need to change how the data handles are done
        //i dont think DeferredReference is goign to play nicely with constexpr, and
        //vectors dont work with constexpr either, which is how the param_pool is currently setup.
        //the parampool could probably be a span tho
        GPUTask Compile(LogicalDevice& device) noexcept;

        //i dont know how to handle command lists that are going to be duplicated
        bool wasCompiled = false;

#if 1 //COMMAND_RECORD_NAMING
        std::string name;
#endif
        //this isnt going to help me setup bindless at all.
        //currently, it would need to be setup externally, and dependencies would need to be tracked externally.
        
        std::vector<CommandInstruction> records{};

        std::vector<void*> deferred_references{};

        //i dont know if i need the Pipeline data for compile time optimization
        DeferredReference<Pipeline*>* BindPipeline();

        //i think i want a descirptor set that contains the details for buffers and images contained
        //im not sure how to do this yet
        //with some further inspection, i want to exclusively use device buffer address
        //DeferredReference<VkDescriptorSet>* BindDescriptor();

        //i need some expanded or manual method to keep track of when buffers are written to in shaders

        DeferredReference<GlobalPushConstant>* Push();

        //this shouldnt be used directly
        DeferredReference<RenderInfo>* BeginRender();
        void EndRender();

        //this needs to be expanded
        //potentially dont even allow this to be called explicitly?
        DeferredReference<PipelineBarrier>* Barrier();

        DeferredReference<LabelParamPack>* BeginLabel() noexcept;
        void EndLabel() noexcept;

        DeferredReference<VertexDrawParamPack>* Draw();
        DeferredReference<IndexDrawParamPack>* DrawIndexed();
        DeferredReference<DispatchParamPack>* Dispatch();

        DeferredReference<ViewportScissorParamPack>* SetViewportScissor();
        DeferredReference<ViewportScissorWithCountParamPack>* SetViewportScissorWithCount();
    };

}//namespace EWE