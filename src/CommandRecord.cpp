#include "EightWinds/RenderGraph/Command/InstructionPointer.h"
#include "EightWinds/RenderGraph/Command/Record.h"
#include "EightWinds/VulkanHeader.h"

#include <cassert>

namespace EWE{
    
    namespace Command{
        void BindCommand(std::vector<Instruction>& records, Instruction::Type cmdType){
            std::size_t paramOffset = 0;
            //if(records.size() > 0){
                //paramOffset = records.back().paramOffset + Instruction::GetParamSize(records.back().type);
            //}
            records.push_back(Instruction{cmdType});
        }
    
        std::size_t Record::CalculateSize() const noexcept{
            std::size_t ret = 0;
            for(auto const& inst : records){
                ret += Instruction::GetParamSize(inst.type);
            }
            return ret;
        }

        #if EWE_DEBUG_BOOL
        bool Record::ValidateInstructions() const{
            int64_t current_if_depth = 0;
            int64_t current_label_depth = 0;
            std::vector<uint32_t> if_command_length{};

            bool pipeline_bound = false;

            bool currently_rendering = false;
            for(auto& rec : records){
                switch(rec.type){
                    case Instruction::Type::If:
                        current_if_depth++;
                        if_command_length.push_back(0);
                        assert(if_command_length.size() == current_if_depth); //unnecessary, sanity check
                        continue;
                    case Instruction::Type::EndIf:
                        assert(if_command_length.size() == current_if_depth); //unnecessary, sanity check
                        current_if_depth--;
                        assert(current_if_depth >= 0);
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
                        assert(current_label_depth >= 0);
                        break;

                    case Instruction::Type::BeginRender:
                        assert(!currently_rendering);
                        currently_rendering = true;
                        break;
                    case Instruction::Type::EndRender:
                        assert(currently_rendering);
                        currently_rendering = false;
                        break;
                    case Instruction::Type::BindPipeline:
                        pipeline_bound = true;
                        break;

                    case Instruction::Type::BindDescriptor:
                    case Instruction::Type::PushConstant:
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

                        assert(pipeline_bound);
                        break;

                    default:
                        break;
                }
                if(if_command_length.size() > 0){
                    if_command_length.back()++;
                }

            }
            assert(current_if_depth == 0);
            assert(current_label_depth == 0);
            return true;
            
        }
        #endif

        /*
        void Record::Compile(GPUTask* constructionPointer, LogicalDevice& logicalDevice, Queue& queue) noexcept {
            assert(!hasBeenCompiled);
            const uint64_t full_data_size = records.back().paramOffset + Instruction::GetParamSize(records.back().type);

            GPUTask ret{logicalDevice, queue};
            ret.commandExecutor.instructions = records;
            ret.commandExecutor.paramPool.resize(full_data_size);
            const std::size_t param_pool_address = reinterpret_cast<std::size_t>(ret.commandExecutor.paramPool.data());
            for(auto& def_ref : deferred_references){
                def_ref->data += param_pool_address;
                //we convert the initial offset to a real pointer into the paramPool
            }
            for(auto& push_off : push_offsets){
                std::size_t temp_addr = reinterpret_cast<std::size_t>(push_off);
                ret.pushTrackers.emplace_back(reinterpret_cast<GlobalPushConstant*>(temp_addr + param_pool_address));
            }
            uint64_t blitIndex = 0;
            for (auto const& inst : records) {
                if (inst.type == Instruction::Type::BeginRender) {
                    ret.renderTracker = new RenderTracker();
                }
                if(inst.type == Instruction::Type::Blit) {
                    auto& blitBack = ret.blitTrackers.emplace_back();
                    blitBack.dstImage.resource = nullptr;
                    blitBack.srcImage.resource = nullptr;
                }
            }

            //all validations will be here
            //theres some non-validation stuff here, like collapsing empty branches
            //maybe split out optimization into a different loop
        #if EWE_DEBUG_BOOL
            assert(ValidateInstructions(records));
        #endif
            hasBeenCompiled = true;

            return ret;
        }
        */

        InstructionPointerAdjuster* Record::AddInstruction(Instruction::Type type, bool external_memory /*= false*/) {
            if(Instruction::GetParamSize(type) == 0){
                records.push_back(
                    Instruction{
                        .type = type,
                        .instruction_pointer = nullptr
                    }
                );
            }
            else{
                records.push_back(
                    Instruction{
                        .type = type,
                        .instruction_pointer = new InstructionPointerAdjuster()
                    }
                );
            }
            return records.back().instruction_pointer;
        }


        InstructionPointer<ParamPack::Viewport>* Record::SetViewport(){
            return reinterpret_cast<
            BindCommand(records, Instruction::Type::DS_Viewport);
            auto deferred_ref = new InstructionPointer<ParamPack::Viewport>();
            deferred_references.push_back(reinterpret_cast<InstructionPointerAdjuster*>(deferred_ref));
            return deferred_ref;
        }
        InstructionPointer<ParamPack::ViewportCount>* Record::SetViewportCount(){
            BindCommand(records, Instruction::Type::DS_ViewportCount);
            auto deferred_ref = new InstructionPointer<ParamPack::ViewportCount>();
            deferred_references.push_back(reinterpret_cast<InstructionPointerAdjuster*>(deferred_ref));
            return deferred_ref;
        }
        InstructionPointer<ParamPack::Scissor>* Record::SetScissor(){
            BindCommand(records, Instruction::Type::DS_Scissor);
            auto deferred_ref = new InstructionPointer<ParamPack::Scissor>();
            deferred_references.push_back(reinterpret_cast<InstructionPointerAdjuster*>(deferred_ref));
            return deferred_ref;
        }
        InstructionPointer<ParamPack::ScissorCount>* Record::SetScissorCount(){
            BindCommand(records, Instruction::Type::DS_ScissorCount);
            auto deferred_ref = new InstructionPointer<ParamPack::ScissorCount>();
            deferred_references.push_back(reinterpret_cast<InstructionPointerAdjuster*>(deferred_ref));
            return deferred_ref;
        }



        InstructionPointer<ParamPack::Pipeline>* Record::BindPipeline(){
            BindCommand(records, Instruction::Type::BindPipeline);
            auto deferred_ref = new InstructionPointer<ParamPack::Pipeline>();
            deferred_references.push_back(reinterpret_cast<InstructionPointerAdjuster*>(deferred_ref));
            return deferred_ref;
        }

        //the plan is to only have the single descriptor set, for all bindless textures
        //void BindDescriptor(VkDescriptorSet set);

        InstructionPointer<GlobalPushConstant_Raw>* Record::Push() {
            //assert a pipeline is binded
            BindCommand(records, Instruction::Type::PushConstant);
            // push_offsets.push_back(reinterpret_cast<GlobalPushConstant_Raw*>());
            InstructionPointer<GlobalPushConstant_Raw>* ret{};
            ret = new InstructionPointer<GlobalPushConstant_Raw>();
            deferred_references.push_back(reinterpret_cast<InstructionPointerAdjuster*>(ret));
            return ret;
        }

        InstructionPointer<VkRenderingInfo>* Record::BeginRender(){
            BindCommand(records, Instruction::Type::BeginRender);
            auto deferred_ref = new InstructionPointer<VkRenderingInfo>();
            deferred_references.push_back(reinterpret_cast<InstructionPointerAdjuster*>(deferred_ref));
            return deferred_ref;
            
        }
        void Record::EndRender(){
            BindCommand(records, Instruction::Type::EndRender);
        }

        void Record::BindDescriptor() {
            BindCommand(records, Instruction::Type::BindDescriptor);
        }

        InstructionPointer<ParamPack::Label>* Record::BeginLabel() noexcept{
            BindCommand(records, Instruction::Type::BeginLabel);
            auto deferred_ref = new InstructionPointer<ParamPack::Label>();
            deferred_references.push_back(reinterpret_cast<InstructionPointerAdjuster*>(deferred_ref));
            return deferred_ref;
        }
        void Record::EndLabel() noexcept{
            BindCommand(records, Instruction::Type::EndLabel);
        }

        InstructionPointer<ParamPack::VertexDraw>* Record::Draw(){
            BindCommand(records, Instruction::Type::Draw);
            auto deferred_ref = new InstructionPointer<ParamPack::VertexDraw>();
            deferred_references.push_back(reinterpret_cast<InstructionPointerAdjuster*>(deferred_ref));
            return deferred_ref;
        }
        InstructionPointer<ParamPack::IndexDraw>* Record::DrawIndexed(){
            BindCommand(records, Instruction::Type::DrawIndexed);
            auto deferred_ref = new InstructionPointer<ParamPack::IndexDraw>();
            deferred_references.push_back(reinterpret_cast<InstructionPointerAdjuster*>(deferred_ref));
            return deferred_ref;
        }
        InstructionPointer<ParamPack::Dispatch>* Record::Dispatch(){
            BindCommand(records, Instruction::Type::Dispatch);
            auto deferred_ref = new InstructionPointer<ParamPack::Dispatch>();
            deferred_references.push_back(reinterpret_cast<InstructionPointerAdjuster*>(deferred_ref));
            return deferred_ref;
        }
        InstructionPointer<ParamPack::DrawMeshTasks>* Record::DrawMeshTasks() {
            BindCommand(records, Instruction::Type::DrawMeshTasks);
            auto deferred_ref = new InstructionPointer<ParamPack::DrawMeshTasks>();
            deferred_references.push_back(reinterpret_cast<InstructionPointerAdjuster*>(deferred_ref));
            return deferred_ref;
        }

        InstructionPointer<ParamPack::DrawIndirect>* Record::DrawIndirect() {
            BindCommand(records, Instruction::Type::DrawIndirect);
            auto deferred_ref = new InstructionPointer<ParamPack::DrawIndirect>();
            deferred_references.push_back(reinterpret_cast<InstructionPointerAdjuster*>(deferred_ref));
            return deferred_ref;
        }
        InstructionPointer<ParamPack::DrawIndexedIndirect>* Record::DrawIndexedIndirect() {
            BindCommand(records, Instruction::Type::DrawIndexedIndirect);
            auto deferred_ref = new InstructionPointer<ParamPack::DrawIndexedIndirect>();
            deferred_references.push_back(reinterpret_cast<InstructionPointerAdjuster*>(deferred_ref));
            return deferred_ref;
        }

        InstructionPointer<ParamPack::DrawMeshTasksIndirect>* Record::DrawMeshTasksIndirect() {
            BindCommand(records, Instruction::Type::DrawMeshTasksIndirect);
            auto deferred_ref = new InstructionPointer<ParamPack::DrawMeshTasksIndirect>();
            deferred_references.push_back(reinterpret_cast<InstructionPointerAdjuster*>(deferred_ref));
            return deferred_ref;
        }
        InstructionPointer<ParamPack::DrawIndirectCount>* Record::DrawIndirectCount() {
            BindCommand(records, Instruction::Type::DrawIndirectCount);
            auto deferred_ref = new InstructionPointer<ParamPack::DrawIndirectCount>();
            deferred_references.push_back(reinterpret_cast<InstructionPointerAdjuster*>(deferred_ref));
            return deferred_ref;
        }
        InstructionPointer<ParamPack::DrawIndexedIndirectCount>* Record::DrawIndexedIndirectCount() {
            BindCommand(records, Command::Instruction::Type::DrawIndexedIndirectCount);
            auto deferred_ref = new InstructionPointer<ParamPack::DrawIndexedIndirectCount>();
            deferred_references.push_back(reinterpret_cast<InstructionPointerAdjuster*>(deferred_ref));
            return deferred_ref;
        }

        void Record::FixDeferred(const PerFlight<std::size_t> pool_address) noexcept {

            std::size_t current_offset = 0;
            for(auto& inst : records){
                if(inst.instruction_pointer->internal){
                    for(uint8_t frame = 0; frame < max_frames_in_flight; frame++){
                        inst.instruction_pointer->data[frame] = pool_address[frame] + current_offset;
                    }
                    inst.instruction_pointer->adjusted = true;
                    current_offset += Instruction::GetParamSize(inst.type);
                }
            }
#if EWE_DEBUG_BOOL
            EWE_ASSERT(current_offset = CalculateSize());
#endif
        }
    }//namespace Command
} //namespace EWE