#include "EightWinds/RenderGraph/Command/Record.h"

#include <cassert>

namespace EWE{
    
    void BindCommand(std::vector<CommandInstruction>& records, CommandInstruction::Type cmdType){
        std::size_t paramOffset = 0;
        if(records.size() > 0){
            paramOffset = records.back().paramOffset + CommandInstruction::GetParamSize(records.back().type);
        }
        records.push_back(CommandInstruction{cmdType, paramOffset});
    }

    //this is not an address, it's an offset into the vector.
    //once compile is called, all of the pointers inside the deferred references will be
    //redirected to point at the real data, within the params pool
    void* GetCurrentOffset(CommandInstruction const& backInstruction) {
        std::size_t offset = backInstruction.paramOffset;
        return reinterpret_cast<void*>(offset);
    }

#if EWE_DEBUG_BOOL
    bool CommandRecord::ValidateInstructions() const{
        int64_t current_if_depth = 0;
        int64_t current_label_depth = 0;
        std::vector<uint32_t> if_command_length{};

        bool pipeline_bound = false;

        bool currently_rendering = false;
        for(auto& rec : records){
            switch(rec.type){
                case CommandInstruction::Type::If:
                    current_if_depth++;
                    if_command_length.push_back(0);
                    assert(if_command_length.size() == current_if_depth); //unnecessary, sanity check
                    continue;
                case CommandInstruction::Type::EndIf:
                    assert(if_command_length.size() == current_if_depth); //unnecessary, sanity check
                    current_if_depth--;
                    assert(current_if_depth >= 0);
                    if(if_command_length.back() == 0){
                        //if its 0, the if and endif instruction can be erased
                    }
                    if_command_length.pop_back();
                    continue;
                case CommandInstruction::Type::BeginLabel:
                    current_label_depth++;
                    break;
                case CommandInstruction::Type::EndLabel:
                    current_label_depth--;
                    assert(current_label_depth >= 0);
                    break;

                case CommandInstruction::Type::BeginRender:
                    assert(!currently_rendering);
                    currently_rendering = true;
                    break;
                case CommandInstruction::Type::EndRender:
                    assert(currently_rendering);
                    currently_rendering = false;
                    break;
                case CommandInstruction::Type::BindPipeline:
                    pipeline_bound = true;
                    break;

                case CommandInstruction::Type::Draw:
                case CommandInstruction::Type::DrawIndexed:
                case CommandInstruction::Type::BindDescriptor:
                case CommandInstruction::Type::PushConstant:
                case CommandInstruction::Type::DS_ViewportScissor:
                case CommandInstruction::Type::DS_ViewportScissorWithCount:

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
    void CommandRecord::Compile(GPUTask* constructionPointer, LogicalDevice& logicalDevice, Queue& queue) noexcept {
        assert(!hasBeenCompiled);
        const uint64_t full_data_size = records.back().paramOffset + CommandInstruction::GetParamSize(records.back().type);

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
            if (inst.type == CommandInstruction::Type::BeginRender) {
                ret.renderTracker = new RenderTracker();
            }
            if(inst.type == CommandInstruction::Type::Blit) {
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

    
    DeferredReference<ViewportScissorParamPack>* CommandRecord::SetViewportScissor(){
        BindCommand(records, CommandInstruction::Type::DS_ViewportScissor);
        auto deferred_ref = new DeferredReference<ViewportScissorParamPack>(GetCurrentOffset(records.back()));
        deferred_references.push_back(reinterpret_cast<DeferredReferenceHelper*>(deferred_ref));
        return deferred_ref;

    }
    DeferredReference<ViewportScissorWithCountParamPack>* CommandRecord::SetViewportScissorWithCount(){
        BindCommand(records, CommandInstruction::Type::DS_ViewportScissorWithCount);
        auto deferred_ref = new DeferredReference<ViewportScissorWithCountParamPack>(GetCurrentOffset(records.back()));
        deferred_references.push_back(reinterpret_cast<DeferredReferenceHelper*>(deferred_ref));
        return deferred_ref;
    }

    DeferredReference<PipelineParamPack>* CommandRecord::BindPipeline(){
        BindCommand(records, CommandInstruction::Type::BindPipeline);
        auto deferred_ref = new DeferredReference<PipelineParamPack>(GetCurrentOffset(records.back()));
        deferred_references.push_back(reinterpret_cast<DeferredReferenceHelper*>(deferred_ref));
        return deferred_ref;
    }

    //the plan is to only have the single descriptor set, for all bindless textures
    //void BindDescriptor(VkDescriptorSet set);
    
    DeferredReference<GlobalPushConstant>* CommandRecord::Push(){
        //assert a pipeline is binded
        BindCommand(records, CommandInstruction::Type::PushConstant);
        push_offsets.push_back(reinterpret_cast<GlobalPushConstant*>(GetCurrentOffset(records.back())));
        auto deferred_ref = new DeferredReference<GlobalPushConstant>(GetCurrentOffset(records.back()));
        deferred_references.push_back(reinterpret_cast<DeferredReferenceHelper*>(deferred_ref));
        return deferred_ref;
    }

    void CommandRecord::BeginRender(){
        BindCommand(records, CommandInstruction::Type::BeginRender);
        //this doesnt return a deferredreference because the pointer is scanned for later
        //it still uses an offset of 8
    }
    void CommandRecord::EndRender(){
        BindCommand(records, CommandInstruction::Type::EndRender);
    }

    void CommandRecord::BindDescriptor() {
        BindCommand(records, CommandInstruction::Type::BindDescriptor);
    }

    DeferredReference<LabelParamPack>* CommandRecord::BeginLabel() noexcept{
        BindCommand(records, CommandInstruction::Type::BeginLabel);
        auto deferred_ref = new DeferredReference<LabelParamPack>(GetCurrentOffset(records.back()));
        deferred_references.push_back(reinterpret_cast<DeferredReferenceHelper*>(deferred_ref));
        return deferred_ref;
    }
    void CommandRecord::EndLabel() noexcept{
        BindCommand(records, CommandInstruction::Type::EndLabel);
    }

    DeferredReference<VertexDrawParamPack>* CommandRecord::Draw(){
        BindCommand(records, CommandInstruction::Type::Draw);
        auto deferred_ref = new DeferredReference<VertexDrawParamPack>(GetCurrentOffset(records.back()));
        deferred_references.push_back(reinterpret_cast<DeferredReferenceHelper*>(deferred_ref));
        return deferred_ref;
    }
    DeferredReference<IndexDrawParamPack>* CommandRecord::DrawIndexed(){
        BindCommand(records, CommandInstruction::Type::DrawIndexed);
        auto deferred_ref = new DeferredReference<IndexDrawParamPack>(GetCurrentOffset(records.back()));
        deferred_references.push_back(reinterpret_cast<DeferredReferenceHelper*>(deferred_ref));
        return deferred_ref;
    }
    DeferredReference<DispatchParamPack>* CommandRecord::Dispatch(){
        BindCommand(records, CommandInstruction::Type::Dispatch);
        auto deferred_ref = new DeferredReference<DispatchParamPack>(GetCurrentOffset(records.back()));
        deferred_references.push_back(reinterpret_cast<DeferredReferenceHelper*>(deferred_ref));
        return deferred_ref;
    }



    void CommandRecord::FixDeferred(const std::size_t pool_address) noexcept {

        for (auto& def_ref : deferred_references) {
            def_ref->data += pool_address;
            def_ref->adjusted = true;
            //we convert the initial offset to a real pointer into the paramPool
        }
    }
} //namespace EWE