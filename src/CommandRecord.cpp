#include "EightWinds/RenderGraph/Command/Record.h"

#include <cassert>

namespace EWE{
    
    namespace Command{
        void BindCommand(std::vector<Instruction>& records, Instruction::Type cmdType){
            std::size_t paramOffset = 0;
            if(records.size() > 0){
                paramOffset = records.back().paramOffset + Instruction::GetParamSize(records.back().type);
            }
            records.push_back(Instruction{cmdType, paramOffset});
        }

        //this is not an address, it's an offset into the vector.
        //once compile is called, all of the pointers inside the deferred references will be
        //redirected to point at the real data, within the params pool
        std::size_t GetCurrentOffset(Instruction const& backInstruction) {
            return backInstruction.paramOffset;
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
                    case Instruction::Type::DS_ViewportScissor:
                    case Instruction::Type::DS_ViewportScissorWithCount:
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


        InstructionPointer<ParamPack::ViewportScissor>* Record::SetViewportScissor(){
            BindCommand(records, Instruction::Type::DS_ViewportScissor);
            auto deferred_ref = new InstructionPointer<ParamPack::ViewportScissor>(GetCurrentOffset(records.back()));
            deferred_references.push_back(reinterpret_cast<InstructionPointerAdjuster*>(deferred_ref));
            return deferred_ref;

        }
        InstructionPointer<ParamPack::ViewportScissorWithCount>* Record::SetViewportScissorWithCount(){
            BindCommand(records, Instruction::Type::DS_ViewportScissorWithCount);
            auto deferred_ref = new InstructionPointer<ParamPack::ViewportScissorWithCount>(GetCurrentOffset(records.back()));
            deferred_references.push_back(reinterpret_cast<InstructionPointerAdjuster*>(deferred_ref));
            return deferred_ref;
        }

        InstructionPointer<ParamPack::Pipeline>* Record::BindPipeline(){
            BindCommand(records, Instruction::Type::BindPipeline);
            auto deferred_ref = new InstructionPointer<ParamPack::Pipeline>(GetCurrentOffset(records.back()));
            deferred_references.push_back(reinterpret_cast<InstructionPointerAdjuster*>(deferred_ref));
            return deferred_ref;
        }

        //the plan is to only have the single descriptor set, for all bindless textures
        //void BindDescriptor(VkDescriptorSet set);

        InstructionPointer<GlobalPushConstant_Raw>* Record::Push() {
            //assert a pipeline is binded
            BindCommand(records, Instruction::Type::PushConstant);
            // push_offsets.push_back(reinterpret_cast<GlobalPushConstant_Raw*>(GetCurrentOffset(records.back())));
            InstructionPointer<GlobalPushConstant_Raw>* ret{};
            ret = new InstructionPointer<GlobalPushConstant_Raw>(GetCurrentOffset(records.back()));
            deferred_references.push_back(reinterpret_cast<InstructionPointerAdjuster*>(ret));
            return ret;
        }

        InstructionPointer<VkRenderingInfo>* Record::BeginRender(){
            BindCommand(records, Instruction::Type::BeginRender);
            auto deferred_ref = new InstructionPointer<VkRenderingInfo>(GetCurrentOffset(records.back()));
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
            auto deferred_ref = new InstructionPointer<ParamPack::Label>(GetCurrentOffset(records.back()));
            deferred_references.push_back(reinterpret_cast<InstructionPointerAdjuster*>(deferred_ref));
            return deferred_ref;
        }
        void Record::EndLabel() noexcept{
            BindCommand(records, Instruction::Type::EndLabel);
        }

        InstructionPointer<ParamPack::VertexDraw>* Record::Draw(){
            BindCommand(records, Instruction::Type::Draw);
            auto deferred_ref = new InstructionPointer<ParamPack::VertexDraw>(GetCurrentOffset(records.back()));
            deferred_references.push_back(reinterpret_cast<InstructionPointerAdjuster*>(deferred_ref));
            return deferred_ref;
        }
        InstructionPointer<ParamPack::IndexDraw>* Record::DrawIndexed(){
            BindCommand(records, Instruction::Type::DrawIndexed);
            auto deferred_ref = new InstructionPointer<ParamPack::IndexDraw>(GetCurrentOffset(records.back()));
            deferred_references.push_back(reinterpret_cast<InstructionPointerAdjuster*>(deferred_ref));
            return deferred_ref;
        }
        InstructionPointer<ParamPack::Dispatch>* Record::Dispatch(){
            BindCommand(records, Instruction::Type::Dispatch);
            auto deferred_ref = new InstructionPointer<ParamPack::Dispatch>(GetCurrentOffset(records.back()));
            deferred_references.push_back(reinterpret_cast<InstructionPointerAdjuster*>(deferred_ref));
            return deferred_ref;
        }
        InstructionPointer<ParamPack::DrawMeshTasks>* Record::DrawMeshTasks() {
            BindCommand(records, Instruction::Type::DrawMeshTasks);
            auto deferred_ref = new InstructionPointer<ParamPack::DrawMeshTasks>(GetCurrentOffset(records.back()));
            deferred_references.push_back(reinterpret_cast<InstructionPointerAdjuster*>(deferred_ref));
            return deferred_ref;
        }

        InstructionPointer<ParamPack::DrawIndirect>* Record::DrawIndirect() {
            BindCommand(records, Instruction::Type::DrawIndirect);
            auto deferred_ref = new InstructionPointer<ParamPack::DrawIndirect>(GetCurrentOffset(records.back()));
            deferred_references.push_back(reinterpret_cast<InstructionPointerAdjuster*>(deferred_ref));
            return deferred_ref;
        }
        InstructionPointer<ParamPack::DrawIndexedIndirect>* Record::DrawIndexedIndirect() {
            BindCommand(records, Instruction::Type::DrawIndexedIndirect);
            auto deferred_ref = new InstructionPointer<ParamPack::DrawIndexedIndirect>(GetCurrentOffset(records.back()));
            deferred_references.push_back(reinterpret_cast<InstructionPointerAdjuster*>(deferred_ref));
            return deferred_ref;
        }

        InstructionPointer<ParamPack::DrawMeshTasksIndirect>* Record::DrawMeshTasksIndirect() {
            BindCommand(records, Instruction::Type::DrawMeshTasksIndirect);
            auto deferred_ref = new InstructionPointer<ParamPack::DrawMeshTasksIndirect>(GetCurrentOffset(records.back()));
            deferred_references.push_back(reinterpret_cast<InstructionPointerAdjuster*>(deferred_ref));
            return deferred_ref;
        }
        InstructionPointer<ParamPack::DrawIndirectCount>* Record::DrawIndirectCount() {
            BindCommand(records, Instruction::Type::DrawIndirectCount);
            auto deferred_ref = new InstructionPointer<ParamPack::DrawIndirectCount>(GetCurrentOffset(records.back()));
            deferred_references.push_back(reinterpret_cast<InstructionPointerAdjuster*>(deferred_ref));
            return deferred_ref;
        }
        InstructionPointer<ParamPack::DrawIndexedIndirectCount>* Record::DrawIndexedIndirectCount() {
            BindCommand(records, Command::Instruction::Type::DrawIndexedIndirectCount);
            auto deferred_ref = new InstructionPointer<ParamPack::DrawIndexedIndirectCount>(GetCurrentOffset(records.back()));
            deferred_references.push_back(reinterpret_cast<InstructionPointerAdjuster*>(deferred_ref));
            return deferred_ref;
        }

        void Record::FixDeferred(const PerFlight<std::size_t> pool_address) noexcept {

            for (auto& def_ref : deferred_references) {
                for (uint8_t i = 0; i < max_frames_in_flight; i++) {
                    def_ref->data[i] += pool_address[i];
                }
                def_ref->adjusted = true;
                //we convert the initial offset to a real pointer into the paramPool
            }
        }
    }//namespace Command
} //namespace EWE