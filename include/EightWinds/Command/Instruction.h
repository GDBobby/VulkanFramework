#pragma once

#include "EightWinds/Command/InstructionType.h"
#include "EightWinds/Command/InstructionPointer.h"
#include "EightWinds/VulkanHeader.h"

#include "EightWinds/Command/ParamPacks.h"
#include "EightWinds/GlobalPushConstant.h"

#include "EightWinds/Reflect/Enum.h"

#include <cstdint>

namespace EWE{

namespace Inst{

    static constexpr uint64_t GetParamSize(Inst::Type type) noexcept{
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
        EWE_UNREACHABLE;
        return 0;
    }
} //namespace Inst

    struct Instruction{

        Inst::Type type;
        std::shared_ptr<InstructionPointerAdjuster> instruction_pointer = nullptr;

        [[nodiscard]] explicit Instruction(Inst::Type _type, InstructionPointerAdjuster* _instruction_pointer)
            : type{_type}, instruction_pointer{_instruction_pointer}
        {}

        Instruction(Instruction const& copySrc) = default;
        [[nodiscard]] Instruction(Instruction&& moveSrc) noexcept = default;
        Instruction& operator=(Instruction const& copySrc) = default;
        Instruction& operator=(Instruction&& moveSrc) noexcept = default;

        ~Instruction(){}

        template<Inst::Type IType>
        static constexpr auto GetVkFunction() noexcept{
            if constexpr(IType == Inst::BindPipeline){
                return vkCmdBindPipeline;
            }
            else if constexpr(IType == Inst::BindDescriptor){
                return vkCmdBindDescriptorSets;
            }
            else if constexpr(IType == Inst::Push){
                return vkCmdPushConstants;
            }
            else if constexpr(IType == Inst::BeginRender){
                return vkCmdBeginRendering;
            }
            else if constexpr(IType == Inst::EndRender){
                return vkCmdEndRendering;
            }
            else if constexpr(IType == Inst::Draw){
                return vkCmdDraw;
            }
            else if constexpr(IType == Inst::DrawIndexed){
                return vkCmdDrawIndexed;
            }
            else if constexpr(IType == Inst::Dispatch){
                return vkCmdDispatch;
            }
            else if constexpr(IType == Inst::DrawMeshTasks){
                return vkCmdDrawMeshTasksEXT;
            }
            else if constexpr(IType == Inst::DrawIndirect){
                return vkCmdDrawIndirect;
            }
            else if constexpr(IType == Inst::DrawIndexedIndirect){
                return vkCmdDrawIndexedIndirect;
            }
            else if constexpr(IType == Inst::DispatchIndirect){
                return vkCmdDispatchIndirect;
            }
            else if constexpr(IType == Inst::DrawMeshTasksIndirect){
                return vkCmdDrawMeshTasksIndirectEXT;
            }
            else if constexpr(IType == Inst::DrawIndirectCount){
                return vkCmdDrawIndirectCount;
            }
            else if constexpr(IType == Inst::DrawIndexedIndirectCount){
                return vkCmdDrawIndexedIndirectCount;
            }
            else if constexpr(IType == Inst::DrawMeshTasksIndirectCount){
                return vkCmdDrawMeshTasksIndirectCountEXT;
            }
            else if constexpr(IType == Inst::DS_Viewport){
                return vkCmdSetViewport;
            }
            else if constexpr(IType == Inst::DS_ViewportCount){
                return vkCmdSetViewportWithCount;
            }
            else if constexpr(IType == Inst::DS_Scissor){
                return vkCmdSetScissor;
            }
            else if constexpr(IType == Inst::DS_ScissorCount){
                return vkCmdSetScissorWithCount;
            }
            else if constexpr(IType == Inst::BeginLabel){
                return nullptr;//this is a logical device function. potentially could make it static in logicaldevice?
            }
            else if constexpr(IType == Inst::EndLabel){
                return nullptr;//this is a logical device function. potentially could make it static in logicaldevice?
            }
            else{
                return nullptr;
            }
        }

        template<Inst::Type IType>
        static constexpr decltype(auto) GetData(InstructionPointerAdjuster* inst_ptr, uint8_t frame) noexcept{
            if constexpr (std::meta::is_complete_type(^^ParamPack<IType>)){
                return *reinterpret_cast<ParamPack<IType>*>(inst_ptr->data[frame]);
            }
            else{
                return nullptr;
            }
        }

        static bool CheckInstructionValidAtBackOf(const std::span<Inst::Type> instructions, Inst::Type itype);
        static std::vector<Inst::Type> GetValidInstructionsAtBackOf(const std::span<const Inst::Type> instructions);

        template<Inst::Type IType>
        struct TypeHelper{
            static constexpr std::size_t param_size = GetParamSize(IType);
            static constexpr auto vulkan_func_ptr = GetVkFunction<IType>();
        };
    };

} //namespace EWE