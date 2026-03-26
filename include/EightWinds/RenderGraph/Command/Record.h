#pragma once

#include "EightWinds/VulkanHeader.h"

#include "EightWinds/RenderGraph/GPUTask.h"

#include "EightWinds/RenderGraph/Command/Instruction.h"
#include "EightWinds/RenderGraph/Command/InstructionPointer.h"

#include "EightWinds/Data/StreamHelper.h"


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
    //i dont think InstructionPointer is goign to play nicely with constexpr, and
    //vectors dont work with constexpr either, which is how the param_pool is currently setup.
    //the parampool could probably be a span tho

    //i need a merge option, so that tasks can be modified more easily
    //and commandrecord can be passed around as a pre-package kind of thing
    
    namespace Command{
        struct Record{
            [[nodiscard]] Record() = default;

            [[nodiscard]] Record(std::string_view file_location);

            Record(Record const&) = delete;
            Record& operator=(Record const&) = delete;
            Record(Record&&) = delete;
            Record& operator=(Record&&) = delete;

            std::string name;

            //i dont know how to handle command lists that are going to be duplicated, or only slightly modified
            //so im going to disable it
            bool hasBeenCompiled = false;

            std::vector<Instruction> records{};

            //a pointer is returned so tha t
            InstructionPointerAdjuster* Add(Instruction::Type type, bool external_memory = false);

            template<Instruction::Type IType>
            decltype(auto) Add(bool external_memory = false){
                auto inst = Add(IType, external_memory);
                 //i dont feel like doign this rn
                if constexpr(IType == Instruction::BindPipeline){
                    return inst->CastTo<ParamPack::Pipeline>();
                }
                else if constexpr(IType == Instruction::BindDescriptor){
                    return;
                }
                else if constexpr(IType == Instruction::Push){
                    return inst->CastTo<GlobalPushConstant_Raw>();
                }
                else if constexpr(IType == Instruction::BeginRender){
                    return inst->CastTo<VkRenderingInfo>();
                }
                else if constexpr(IType == Instruction::EndRender){
                    return;
                }
                else if constexpr(IType == Instruction::Draw){
                    return inst->CastTo<ParamPack::VertexDraw>();
                }
                else if constexpr(IType == Instruction::DrawIndexed){
                    return inst->CastTo<ParamPack::IndexDraw>();
                }
                else if constexpr(IType == Instruction::Dispatch){
                    return inst->CastTo<ParamPack::Dispatch>();
                }
                else if constexpr(IType == Instruction::DrawMeshTasks){
                    return inst->CastTo<ParamPack::DrawMeshTasks>();
                }
                else if constexpr(IType == Instruction::DrawIndirect){
                    return inst->CastTo<ParamPack::DrawIndirect>();
                }
                else if constexpr(IType == Instruction::DrawIndexedIndirect){
                    return inst->CastTo<ParamPack::DrawIndirect>();
                }
                else if constexpr(IType == Instruction::DispatchIndirect){
                    return inst->CastTo<ParamPack::DispatchIndirect>();
                }
                else if constexpr(IType == Instruction::DrawMeshTasksIndirect){
                    return inst->CastTo<ParamPack::DrawIndirect>();
                }
                else if constexpr(IType == Instruction::DrawIndirectCount){
                    return inst->CastTo<ParamPack::DrawIndirectCount>();
                }
                else if constexpr(IType == Instruction::DrawIndexedIndirectCount){
                    return inst->CastTo<ParamPack::DrawIndirectCount>();
                }
                else if constexpr(IType == Instruction::DrawMeshTasksIndirectCount){
                    return inst->CastTo<ParamPack::DrawIndirectCount>();
                }
                else if constexpr(IType == Instruction::DS_Viewport){
                    return inst->CastTo<ParamPack::Viewport>();
                }
                else if constexpr(IType == Instruction::DS_ViewportCount){
                    return inst->CastTo<ParamPack::ViewportCount>();
                }
                else if constexpr(IType == Instruction::DS_Scissor){
                    return inst->CastTo<ParamPack::Scissor>();
                }
                else if constexpr(IType == Instruction::DS_ScissorCount){
                    return inst->CastTo<ParamPack::ScissorCount>();
                }
                else if constexpr(IType == Instruction::BeginLabel){
                    return inst->CastTo<ParamPack::Label>();
                }
                else if constexpr(IType == Instruction::EndLabel){
                    return;//this is a logical device function. potentially could make it static in logicaldevice?
                }
                else{
                    return;
                }
            }

            std::size_t CalculateSize() const noexcept;
            void FixDeferred(const PerFlight<std::size_t> pool_address) noexcept;
#if EWE_DEBUG_BOOL
            bool ValidateInstructions() const;
#endif

            static void WriteInstructions(std::string_view file_location, const std::span<const Instruction::Type> instructions);
            void WriteInstructions(std::string_view file_location);
            static RuntimeArray<Instruction::Type> ReadInstructions(std::string_view file_location);
        };
    }//namespace Command
}//namespace EWE