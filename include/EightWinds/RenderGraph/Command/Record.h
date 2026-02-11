#pragma once

#include "EightWinds/VulkanHeader.h"

#include "EightWinds/RenderGraph/GPUTask.h"

#include "EightWinds/RenderGraph/Command/Instruction.h"
#include "EightWinds/RenderGraph/Command/DeferredReference.h"

#include <array>
#include <string>

namespace EWE{
    //forward declared just to save a bit of compile time
    //the implementation doesnt matter here
    struct Pipeline;
    struct PipelineBarrier;
    struct CommandBuffer;
    struct GlobalPushConstant_Raw;
    struct RenderInfo;
    struct LogicalDevice;

    //if i want compile time optimization, i need to change how the data handles are done
    //i dont think DeferredReference is goign to play nicely with constexpr, and
    //vectors dont work with constexpr either, which is how the param_pool is currently setup.
    //the parampool could probably be a span tho

    //i need a merge option, so that tasks can be modified more easily
    //and commandrecord can be passed around as a pre-package kind of thing
    
    namespace Command{
        struct Record{
            Record() = default;
            Record(Record const&) = delete;
            Record& operator=(Record const&) = delete;
            Record(Record&&) = delete;
            Record& operator=(Record&&) = delete;

            //i dont know how to handle command lists that are going to be duplicated, or only slightly modified
            //so im going to disable it
            bool hasBeenCompiled = false;

            std::vector<Instruction> records{};
            std::vector<DeferredReferenceHelper*> deferred_references{};
            //std::vector<GlobalPushConstant_Raw*> push_offsets{};

            //i dont know if i need the Pipeline data for compile time optimization
            DeferredReference<ParamPack::Pipeline>* BindPipeline();

            //void return only works if i FORCE the user to use device buffer addresses and bindless textures from a push constant
            //and only the global bindless descriptor set is allowed (found in logicalDevice)
            void BindDescriptor();

            //i need some expanded or manual method to keep track of when buffers are written to in shaders

            DeferredReference<GlobalPushConstant_Raw>* Push();

            //this shouldnt be used directly
            DeferredReference<VkRenderingInfo>* BeginRender();
            void EndRender();

            DeferredReference<ParamPack::Label>* BeginLabel() noexcept;
            void EndLabel() noexcept;

            DeferredReference<ParamPack::VertexDraw>* Draw();
            DeferredReference<ParamPack::IndexDraw>* DrawIndexed();
            DeferredReference<ParamPack::Dispatch>* Dispatch();
            DeferredReference<ParamPack::DrawMeshTasks>* DrawMeshTasks();
            
            DeferredReference<ParamPack::DrawIndirect>* DrawIndirect();
            DeferredReference<ParamPack::DrawIndexedIndirect>* DrawIndexedIndirect();
            DeferredReference<ParamPack::DispatchIndirect>* DispatchIndirect();
            DeferredReference<ParamPack::DrawMeshTasksIndirect>* DrawMeshTasksIndirect();
            
            DeferredReference<ParamPack::DrawIndirectCount>* DrawIndirectCount();
            DeferredReference<ParamPack::DrawIndexedIndirectCount>* DrawIndexedIndirectCount();
            DeferredReference<ParamPack::DrawMeshTasksIndirectCount>* DrawMeshIndirect();

            DeferredReference<ParamPack::ViewportScissor>* SetViewportScissor();
            DeferredReference<ParamPack::ViewportScissorWithCount>* SetViewportScissorWithCount();

            void FixDeferred(const PerFlight<std::size_t> pool_address) noexcept;
#if EWE_DEBUG_BOOL
            bool ValidateInstructions() const;
#endif
        };
    }//namespace Command
}//namespace EWE