#include "EightWinds/RenderGraph/Command/Instruction.h"

#include "EightWinds/Pipeline/PipelineBase.h"
#include "EightWinds/GlobalPushConstant.h"
#include "EightWinds/Backend/RenderInfo.h"

namespace EWE{

    uint64_t Instruction::GetParamSize(Inst::Type type) noexcept{
        static constexpr auto type_mems = std::define_static_array(std::meta::enumerators_of(^^Inst::Type));

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
        template for(constexpr auto type_mem : type_mems){
            if ([:type_mem:] == type){
                if constexpr(std::meta::is_complete_type(^^ParamPack<([:type_mem:])>)){
                    return sizeof(ParamPack<([:type_mem:])>);
                }
                else{
                    return 0;
                }
            }
        }
#pragma GCC diagnostic pop
    }

    bool Instruction::CheckInstructionValidAtBackOf(const std::span<Inst::Type> instructions, Inst::Type itype){
        int64_t current_if_depth = 0;
        int64_t current_label_depth = 0;
        std::vector<uint32_t> if_command_length{};

        bool pipeline_bound = false;

        bool currently_rendering = false;
        for(auto& inst : instructions){
            switch(inst){
                case Inst::Type::If:
                    current_if_depth++;
                    if_command_length.push_back(0);
                    EWE_ASSERT(if_command_length.size() == current_if_depth); //unnecessary, sanity check
                    continue;
                case Inst::Type::EndIf:
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
                case Inst::Type::BeginLabel:
                    current_label_depth++;
                    break;
                case Inst::Type::EndLabel:
                    current_label_depth--;
                    if(current_label_depth < 0){
                        return false;
                    }
                    break;

                case Inst::Type::BeginRender:
                    if(currently_rendering){
                        return false;
                    }
                    currently_rendering = true;
                    break;
                case Inst::Type::EndRender:
                    if(!currently_rendering){
                        return false;
                    }
                    currently_rendering = false;
                    break;
                case Inst::Type::BindPipeline:
                    pipeline_bound = true;
                    break;

                case Inst::Type::BindDescriptor:
                case Inst::Type::Push:
                case Inst::Type::DS_Viewport:
                case Inst::Type::DS_ViewportCount:
                case Inst::Type::DS_Scissor:
                case Inst::Type::DS_ScissorCount:
                case Inst::Type::Draw:
                case Inst::Type::DrawIndexed:
                case Inst::Type::Dispatch:
                case Inst::Type::DrawMeshTasks:
                case Inst::Type::DrawIndirect:
                case Inst::Type::DrawIndexedIndirect:
                case Inst::Type::DispatchIndirect:
                case Inst::Type::DrawMeshTasksIndirect:
                case Inst::Type::DrawIndirectCount:
                case Inst::Type::DrawIndexedIndirectCount:
                case Inst::Type::DrawMeshTasksIndirectCount:

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
    std::vector<Inst::Type> Instruction::GetValidInstructionsAtBackOf(const std::span<const Inst::Type> instructions){
        std::vector<Inst::Type> ret{};
        static constexpr auto enum_data = Reflect::Enum::enum_data<Inst::Type>;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
        template for(constexpr auto inst : enum_data){
            if(CheckInstructionValidAtBackOf(ret, inst.value)){
                ret.push_back(inst.value);
            }
        }
#pragma GCC diagnostic pop
        return ret;
    }
} //namespace EWE