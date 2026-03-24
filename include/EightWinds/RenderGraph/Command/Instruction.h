#pragma once

#include "EightWinds/RenderGraph/Command/InstructionPointer.h"
#include "EightWinds/VulkanHeader.h"

#include "EightWinds/RenderGraph/Command/ParamPacks.h"
#include "EightWinds/GlobalPushConstant.h"

#include "EightWinds/Reflect/Enum.h"

#include <cstdint>
#include <vulkan/vulkan_core.h>

namespace EWE{
    struct Instruction{

        enum Type : uint8_t { 
            BindPipeline,

            //descriptor needs to be expanded to handle images and buffers
            BindDescriptor,

            Push, 

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
            DS_Viewport,
            DS_ViewportCount,
            DS_Scissor,
            DS_ScissorCount,

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
        /*
        
            * i could create some powerful features if i tie these 1:1 with vulkan api functions
            * however, i'd lose the ability to create an abstraction layer most likely
            * the question would be, am I ever going to ship to mobile (vk 1.2) or console?
            * ill put it off
                
            std::array<PFN_VK, Type::COUNT>{
                vkCmdBindPipeline,
                vkCmdBindDescrptor,
                vkCmdPush
                ...
            };
        */

        static constexpr uint64_t GetParamSize(Instruction::Type type) noexcept{
            switch (type) {
                case Type::BindPipeline: return sizeof(ParamPack::Pipeline);
                case Type::BindDescriptor: return sizeof(VkDescriptorSet);
                case Type::Push: return sizeof(GlobalPushConstant_Raw);
                case Type::BeginRender: return sizeof(VkRenderingInfo);
                case Type::EndRender: return 0;
                
                case Type::Draw: return sizeof(ParamPack::VertexDraw);
                case Type::DrawIndexed: return sizeof(ParamPack::IndexDraw);
                case Type::Dispatch: return sizeof(ParamPack::Dispatch);
                case Type::DrawMeshTasks: return sizeof(ParamPack::DrawMeshTasks);
                //raytracing here
                
                case Type::DrawIndirect: return sizeof(ParamPack::DrawIndirect);
                case Type::DrawIndexedIndirect: return sizeof(ParamPack::DrawIndirect);
                case Type::DispatchIndirect: return sizeof(ParamPack::DispatchIndirect);
                case Type::DrawMeshTasksIndirect: return sizeof(ParamPack::DrawIndirect);
                
                case Type::DrawIndirectCount: return sizeof(ParamPack::DrawIndirectCount);
                case Type::DrawIndexedIndirectCount: return sizeof(ParamPack::DrawIndirectCount);
                case Type::DrawMeshTasksIndirectCount: return sizeof(ParamPack::DrawIndirectCount);
                //case Type::PipelineBarrier: return 0;
                case Type::DS_Viewport: return sizeof(ParamPack::Viewport);
                case Type::DS_Scissor: return sizeof(ParamPack::Scissor);
                case Type::DS_ViewportCount: return sizeof(ParamPack::ViewportCount);
                case Type::DS_ScissorCount: return sizeof(ParamPack::ScissorCount);
                case Type::BeginLabel: return sizeof(ParamPack::Label);
                case Type::EndLabel: return 0;
                case Type::If: return sizeof(bool);
                //other loop controls
                case Type::EndIf: return 0;
                default: return 0;

                //bunch of dynamic state that i havent got to yet
            }
            //EWE_UNREACHABLE;
            return 0;
        }

        template<Type IType>
        static constexpr auto GetVkFunction() noexcept{
            if constexpr(IType == Instruction::BindPipeline){
                return vkCmdBindPipeline;
            }
            else if constexpr(IType == Instruction::BindDescriptor){
                return vkCmdBindDescriptorSets;
            }
            else if constexpr(IType == Instruction::Push){
                return vkCmdPushConstants;
            }
            else if constexpr(IType == Instruction::BeginRender){
                return vkCmdBeginRendering;
            }
            else if constexpr(IType == Instruction::EndRender){
                return vkCmdEndRendering;
            }
            else if constexpr(IType == Instruction::Draw){
                return vkCmdDraw;
            }
            else if constexpr(IType == Instruction::DrawIndexed){
                return vkCmdDrawIndexed;
            }
            else if constexpr(IType == Instruction::Dispatch){
                return vkCmdDispatch;
            }
            else if constexpr(IType == Instruction::DrawMeshTasks){
                return vkCmdDrawMeshTasksEXT;
            }
            else if constexpr(IType == Instruction::DrawIndirect){
                return vkCmdDrawIndirect;
            }
            else if constexpr(IType == Instruction::DrawIndexedIndirect){
                return vkCmdDrawIndexedIndirect;
            }
            else if constexpr(IType == Instruction::DispatchIndirect){
                return vkCmdDispatchIndirect;
            }
            else if constexpr(IType == Instruction::DrawMeshTasksIndirect){
                return vkCmdDrawMeshTasksIndirectEXT;
            }
            else if constexpr(IType == Instruction::DrawIndirectCount){
                return vkCmdDrawIndirectCount;
            }
            else if constexpr(IType == Instruction::DrawIndexedIndirectCount){
                return vkCmdDrawIndexedIndirectCount;
            }
            else if constexpr(IType == Instruction::DrawMeshTasksIndirectCount){
                return vkCmdDrawMeshTasksIndirectCountEXT;
            }
            else if constexpr(IType == Instruction::DS_Viewport){
                return vkCmdSetViewport;
            }
            else if constexpr(IType == Instruction::DS_ViewportCount){
                return vkCmdSetViewportWithCount;
            }
            else if constexpr(IType == Instruction::DS_Scissor){
                return vkCmdSetScissor;
            }
            else if constexpr(IType == Instruction::DS_ScissorCount){
                return vkCmdSetScissorWithCount;
            }
            else if constexpr(IType == Instruction::BeginLabel){
                return nullptr;//this is a logical device function. potentially could make it static in logicaldevice?
            }
            else if constexpr(IType == Instruction::EndLabel){
                return nullptr;//this is a logical device function. potentially could make it static in logicaldevice?
            }
            else{
                return nullptr;
            }
        }

        template<Type IType>
        static constexpr decltype(auto) GetData(InstructionPointerAdjuster* inst_ptr, uint8_t frame) noexcept{
            if constexpr(IType == Instruction::BindPipeline){
                return *reinterpret_cast<ParamPack::Pipeline*>(inst_ptr->data[frame]);
            }
            else if constexpr(IType == Instruction::BindDescriptor){
                return nullptr;
            }
            else if constexpr(IType == Instruction::Push){
                return *reinterpret_cast<GlobalPushConstant_Raw*>(inst_ptr->data[frame]);
            }
            else if constexpr(IType == Instruction::BeginRender){
                return *reinterpret_cast<VkRenderingInfo*>(inst_ptr->data[frame]);
            }
            else if constexpr(IType == Instruction::EndRender){
                return nullptr;
            }
            else if constexpr(IType == Instruction::Draw){
                return *reinterpret_cast<ParamPack::VertexDraw*>(inst_ptr->data[frame]);
            }
            else if constexpr(IType == Instruction::DrawIndexed){
                return *reinterpret_cast<ParamPack::IndexDraw*>(inst_ptr->data[frame]);
            }
            else if constexpr(IType == Instruction::Dispatch){
                return *reinterpret_cast<ParamPack::Dispatch*>(inst_ptr->data[frame]);
            }
            else if constexpr(IType == Instruction::DrawMeshTasks){
                return *reinterpret_cast<ParamPack::DrawMeshTasks*>(inst_ptr->data[frame]);
            }
            else if constexpr(IType == Instruction::DrawIndirect){
                return *reinterpret_cast<ParamPack::DrawIndirect*>(inst_ptr->data[frame]);
            }
            else if constexpr(IType == Instruction::DrawIndexedIndirect){
                return *reinterpret_cast<ParamPack::DrawIndirect*>(inst_ptr->data[frame]);
            }
            else if constexpr(IType == Instruction::DispatchIndirect){
                return *reinterpret_cast<ParamPack::DispatchIndirect*>(inst_ptr->data[frame]);
            }
            else if constexpr(IType == Instruction::DrawMeshTasksIndirect){
                return *reinterpret_cast<ParamPack::DrawIndirect*>(inst_ptr->data[frame]);
            }
            else if constexpr(IType == Instruction::DrawIndirectCount){
                return *reinterpret_cast<ParamPack::DrawIndirectCount*>(inst_ptr->data[frame]);
            }
            else if constexpr(IType == Instruction::DrawIndexedIndirectCount){
                return *reinterpret_cast<ParamPack::DrawIndirectCount*>(inst_ptr->data[frame]);
            }
            else if constexpr(IType == Instruction::DrawMeshTasksIndirectCount){
                return *reinterpret_cast<ParamPack::DrawIndirectCount*>(inst_ptr->data[frame]);
            }
            else if constexpr(IType == Instruction::DS_Viewport){
                return *reinterpret_cast<ParamPack::Viewport*>(inst_ptr->data[frame]);
            }
            else if constexpr(IType == Instruction::DS_ViewportCount){
                return *reinterpret_cast<ParamPack::ViewportCount*>(inst_ptr->data[frame]);
            }
            else if constexpr(IType == Instruction::DS_Scissor){
                return *reinterpret_cast<ParamPack::Scissor*>(inst_ptr->data[frame]);
            }
            else if constexpr(IType == Instruction::DS_ScissorCount){
                return *reinterpret_cast<ParamPack::ScissorCount*>(inst_ptr->data[frame]);
            }
            else if constexpr(IType == Instruction::BeginLabel){
                return *reinterpret_cast<ParamPack::Label*>(inst_ptr->data[frame]);
            }
            else if constexpr(IType == Instruction::EndLabel){
                return nullptr;//this is a logical device function. potentially could make it static in logicaldevice?
            }
            else{
                return nullptr;
            }
        }

        static constexpr bool CheckInstructionValidAtBackOf(const std::span<Instruction::Type> instructions, Instruction::Type itype){
            int64_t current_if_depth = 0;
            int64_t current_label_depth = 0;
            std::vector<uint32_t> if_command_length{};

            bool pipeline_bound = false;

            bool currently_rendering = false;
            for(auto& inst : instructions){
                switch(inst){
                    case Instruction::Type::If:
                        current_if_depth++;
                        if_command_length.push_back(0);
                        EWE_ASSERT(if_command_length.size() == current_if_depth); //unnecessary, sanity check
                        continue;
                    case Instruction::Type::EndIf:
                        EWE_ASSERT(if_command_length.size() == current_if_depth); //unnecessary, sanity check
                        current_if_depth--;
                        if(current_if_depth < 0){
                            return false;
                        }
                        if(if_command_length.back() == 0){
                            //if its 0, the if and endif instruction can be erased
                        }
                        if_command_length.pop_back();
                        continue;
                    case Instruction::Type::BeginLabel:
                        current_label_depth++;
                        break;
                    case Instruction::Type::EndLabel:
                        current_label_depth--;
                        if(current_label_depth < 0){
                            return false;
                        }
                        break;

                    case Instruction::Type::BeginRender:
                        if(currently_rendering){
                            return false;
                        }
                        currently_rendering = true;
                        break;
                    case Instruction::Type::EndRender:
                        if(!currently_rendering){
                            return false;
                        }
                        currently_rendering = false;
                        break;
                    case Instruction::Type::BindPipeline:
                        pipeline_bound = true;
                        break;

                    case Instruction::Type::BindDescriptor:
                    case Instruction::Type::Push:
                    case Instruction::Type::DS_Viewport:
                    case Instruction::Type::DS_ViewportCount:
                    case Instruction::Type::DS_Scissor:
                    case Instruction::Type::DS_ScissorCount:
                    case Instruction::Type::Draw:
                    case Instruction::Type::DrawIndexed:
                    case Instruction::Type::Dispatch:
                    case Instruction::Type::DrawMeshTasks:
                    case Instruction::Type::DrawIndirect:
                    case Instruction::Type::DrawIndexedIndirect:
                    case Instruction::Type::DispatchIndirect:
                    case Instruction::Type::DrawMeshTasksIndirect:
                    case Instruction::Type::DrawIndirectCount:
                    case Instruction::Type::DrawIndexedIndirectCount:
                    case Instruction::Type::DrawMeshTasksIndirectCount:

                        if(!pipeline_bound){
                            return false;
                        }
                        break;

                    default:
                        break;
                }
                if(if_command_length.size() > 0){
                    if_command_length.back()++;
                }

            }
            return true;
        }
        static constexpr std::vector<Type> GetValidInstructionsAtBackOf(const std::span<const Instruction::Type> instructions){
            std::vector<Type> ret{};
            static constexpr auto enum_data = Reflect::Enum::enum_data<Instruction::Type>;

#if defined(__clang__) || defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wshadow"
#elif defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable : 4456 4457 4458 4459)
#endif
            template for(constexpr auto inst : enum_data){
                if(CheckInstructionValidAtBackOf(ret, inst.value)){
                    ret.push_back(inst.value);
                }
            }
#if defined(__clang__) || defined(__GNUC__)
    #pragma GCC diagnostic pop
#elif defined(_MSC_VER)
    #pragma warning(pop)
#endif
            return ret;
        }

        template<Type IType>
        struct TypeHelper{
            static constexpr std::size_t param_size = GetParamSize(IType);
            static constexpr auto vulkan_func_ptr = GetVkFunction<IType>();
        };

        Instruction::Type type;
        InstructionPointerAdjuster* instruction_pointer = nullptr;

        [[nodiscard]] explicit Instruction(Instruction::Type _type, InstructionPointerAdjuster* _instruction_pointer)
            : type{_type}, instruction_pointer{_instruction_pointer}
        {}

        Instruction(Instruction const& copySrc) = delete;
        //: type{copySrc.type}, 
        //    instruction_pointer{copySrc.instruction_pointer}
        //{
        //    copySrc.instruction_pointer = nullptr;
        //}
        [[nodiscard]] Instruction(Instruction&& moveSrc) noexcept
        : type{moveSrc.type}, 
            instruction_pointer{moveSrc.instruction_pointer}
        {
            moveSrc.instruction_pointer = nullptr;
        }

        Instruction& operator=(Instruction const& copySrc) = delete;
        //{
        //    type = copySrc.type;
        //    instruction_pointer = copySrc.instruction_pointer;
        //    copySrc.instruction_pointer = nullptr;
        //    return *this;
        //}
        Instruction& operator=(Instruction&& moveSrc) noexcept{
            type = moveSrc.type;
            instruction_pointer = moveSrc.instruction_pointer;
            moveSrc.instruction_pointer = nullptr;
            return *this;
        }

        ~Instruction(){
            if(instruction_pointer != nullptr){
                //idk if this is good or not
                //most likely, i need to cast it then delete anyways
                delete instruction_pointer;
            }
        }

        
    };

} //namespace EWE