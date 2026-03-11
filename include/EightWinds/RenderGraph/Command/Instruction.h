#pragma once

#include "EightWinds/RenderGraph/Command/InstructionPointer.h"
#include "EightWinds/VulkanHeader.h"

#include "EightWinds/RenderGraph/Command/ParamPacks.h"
#include "EightWinds/GlobalPushConstant.h"

#include <cstdint>
#include <vulkan/vulkan_core.h>

namespace EWE{
    namespace Command{
        struct Instruction{

            enum Type : uint8_t { 
                BindPipeline,

                //descriptor needs to be expanded to handle images and buffers
                BindDescriptor,

                PushConstant, 

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
                    case Type::PushConstant: return sizeof(GlobalPushConstant_Raw);
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

                    //bunch of dynamic state that i havent got to yet
                }
                //EWE_UNREACHABLE;
                return 0;
            }

            template<Type _Type>
            static constexpr auto GetVkFunction() noexcept{
                if constexpr(_Type == Instruction::BindPipeline){
                    return vkCmdBindPipeline;
                }
                else if constexpr(_Type == Instruction::BindDescriptor){
                    return vkCmdBindDescriptorSets;
                }
                else if constexpr(_Type == Instruction::PushConstant){
                    return vkCmdPushConstants;
                }
                else if constexpr(_Type == Instruction::BeginRender){
                    return vkCmdBeginRendering;
                }
                else if constexpr(_Type == Instruction::EndRender){
                    return vkCmdEndRendering;
                }
                else if constexpr(_Type == Instruction::Draw){
                    return vkCmdDraw;
                }
                else if constexpr(_Type == Instruction::DrawIndexed){
                    return vkCmdDrawIndexed;
                }
                else if constexpr(_Type == Instruction::Dispatch){
                    return vkCmdDispatch;
                }
                else if constexpr(_Type == Instruction::DrawMeshTasks){
                    return vkCmdDrawMeshTasksEXT;
                }
                else if constexpr(_Type == Instruction::DrawIndirect){
                    return vkCmdDrawIndirect;
                }
                else if constexpr(_Type == Instruction::DrawIndexedIndirect){
                    return vkCmdDrawIndexedIndirect;
                }
                else if constexpr(_Type == Instruction::DispatchIndirect){
                    return vkCmdDispatchIndirect;
                }
                else if constexpr(_Type == Instruction::DrawMeshTasksIndirect){
                    return vkCmdDrawMeshTasksIndirectEXT;
                }
                else if constexpr(_Type == Instruction::DrawIndirectCount){
                    return vkCmdDrawIndirectCount;
                }
                else if constexpr(_Type == Instruction::DrawIndexedIndirectCount){
                    return vkCmdDrawIndexedIndirectCount;
                }
                else if constexpr(_Type == Instruction::DrawMeshTasksIndirectCount){
                    return vkCmdDrawMeshTasksIndirectCountEXT;
                }
                else if constexpr(_Type == Instruction::DS_Viewport){
                    return vkCmdSetViewport;
                }
                else if constexpr(_Type == Instruction::DS_ViewportCount){
                    return vkCmdSetViewportWithCount;
                }
                else if constexpr(_Type == Instruction::DS_Scissor){
                    return vkCmdSetScissor;
                }
                else if constexpr(_Type == Instruction::DS_ScissorCount){
                    return vkCmdSetScissorWithCount;
                }
                else if constexpr(_Type == Instruction::BeginLabel){
                    return nullptr;//this is a logical device function. potentially could make it static in logicaldevice?
                }
                else if constexpr(_Type == Instruction::EndLabel){
                    return nullptr;//this is a logical device function. potentially could make it static in logicaldevice?
                }
                else{
                    return nullptr;
                }
            }

            template<Type _Type>
            static constexpr decltype(auto) GetData(InstructionPointerAdjuster* inst_ptr, uint8_t frame) noexcept{
                if constexpr(_Type == Instruction::BindPipeline){
                    return *reinterpret_cast<ParamPack::Pipeline*>(inst_ptr->data[frame]);
                }
                else if constexpr(_Type == Instruction::BindDescriptor){
                    return nullptr;
                }
                else if constexpr(_Type == Instruction::PushConstant){
                    return *reinterpret_cast<GlobalPushConstant_Raw*>(inst_ptr->data[frame]);
                }
                else if constexpr(_Type == Instruction::BeginRender){
                    return *reinterpret_cast<VkRenderingInfo*>(inst_ptr->data[frame]);
                }
                else if constexpr(_Type == Instruction::EndRender){
                    return nullptr;
                }
                else if constexpr(_Type == Instruction::Draw){
                    return *reinterpret_cast<ParamPack::VertexDraw*>(inst_ptr->data[frame]);
                }
                else if constexpr(_Type == Instruction::DrawIndexed){
                    return *reinterpret_cast<ParamPack::IndexDraw*>(inst_ptr->data[frame]);
                }
                else if constexpr(_Type == Instruction::Dispatch){
                    return *reinterpret_cast<ParamPack::Dispatch*>(inst_ptr->data[frame]);
                }
                else if constexpr(_Type == Instruction::DrawMeshTasks){
                    return *reinterpret_cast<ParamPack::DrawMeshTasks*>(inst_ptr->data[frame]);
                }
                else if constexpr(_Type == Instruction::DrawIndirect){
                    return *reinterpret_cast<ParamPack::DrawIndirect*>(inst_ptr->data[frame]);
                }
                else if constexpr(_Type == Instruction::DrawIndexedIndirect){
                    return *reinterpret_cast<ParamPack::DrawIndirect*>(inst_ptr->data[frame]);
                }
                else if constexpr(_Type == Instruction::DispatchIndirect){
                    return *reinterpret_cast<ParamPack::DispatchIndirect*>(inst_ptr->data[frame]);
                }
                else if constexpr(_Type == Instruction::DrawMeshTasksIndirect){
                    return *reinterpret_cast<ParamPack::DrawIndirect*>(inst_ptr->data[frame]);
                }
                else if constexpr(_Type == Instruction::DrawIndirectCount){
                    return *reinterpret_cast<ParamPack::DrawIndirectCount*>(inst_ptr->data[frame]);
                }
                else if constexpr(_Type == Instruction::DrawIndexedIndirectCount){
                    return *reinterpret_cast<ParamPack::DrawIndirectCount*>(inst_ptr->data[frame]);
                }
                else if constexpr(_Type == Instruction::DrawMeshTasksIndirectCount){
                    return *reinterpret_cast<ParamPack::DrawIndirectCount*>(inst_ptr->data[frame]);
                }
                else if constexpr(_Type == Instruction::DS_Viewport){
                    return *reinterpret_cast<ParamPack::Viewport*>(inst_ptr->data[frame]);
                }
                else if constexpr(_Type == Instruction::DS_ViewportCount){
                    return *reinterpret_cast<ParamPack::ViewportCount*>(inst_ptr->data[frame]);
                }
                else if constexpr(_Type == Instruction::DS_Scissor){
                    return *reinterpret_cast<ParamPack::Scissor*>(inst_ptr->data[frame]);
                }
                else if constexpr(_Type == Instruction::DS_ScissorCount){
                    return *reinterpret_cast<ParamPack::ScissorCount*>(inst_ptr->data[frame]);
                }
                else if constexpr(_Type == Instruction::BeginLabel){
                    return *reinterpret_cast<ParamPack::Label*>(inst_ptr->data[frame]);
                }
                else if constexpr(_Type == Instruction::EndLabel){
                    return nullptr;//this is a logical device function. potentially could make it static in logicaldevice?
                }
                else{
                    return nullptr;
                }
            }


            template<Type _Type>
            struct TypeHelper{
                static constexpr std::size_t param_size = GetParamSize(_Type);
                static constexpr auto vulkan_func_ptr = GetVkFunction<_Type>();
            };


                

            Instruction::Type type;
            InstructionPointerAdjuster* instruction_pointer = nullptr;

            [[nodiscard]] explicit Instruction(Instruction::Type type)
                : type(type)
            {
                
            }
            ~Instruction(){
                if(instruction_pointer != nullptr){
                    delete instruction_pointer;
                }
            }

            
        };
    } //namespace Command

} //namespace EWE