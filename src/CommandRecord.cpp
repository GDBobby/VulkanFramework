#include "EightWinds/Command/Record.h"

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

    struct OffsetHelper{
        std::size_t data;
    };

#if EWE_DEBUG_BOOL
    bool ValidateInstructions(std::vector<CommandInstruction> const& records){
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
    
    GPUTask CommandRecord::Compile(LogicalDevice& logicalDevice) noexcept {
        uint64_t full_data_size = records.back().paramOffset + CommandInstruction::GetParamSize(records.back().type);

        GPUTask ret{logicalDevice};
        ret.commandExecutor.instructions = records;
        ret.commandExecutor.paramPool.resize(full_data_size);
        for(auto& def_ref : deferred_references){
            OffsetHelper* offHelp = reinterpret_cast<OffsetHelper*>(def_ref);
            offHelp->data += reinterpret_cast<uint64_t>(ret.commandExecutor.paramPool.data());
            //we convert the initial offset to a real pointer into the paramPool
        }

        //all validations will be here
        //theres some non-validation stuff here, like collapsing empty branches
        //maybe split out optimization into a different loop
#if EWE_DEBUG_BOOL
        assert(ValidateInstructions(records));
#endif

        return ret;
    }

    
    DeferredReference<ViewportScissorParamPack>* CommandRecord::SetViewportScissor(){
        BindCommand(records, CommandInstruction::Type::DS_ViewportScissor);
        auto deferred_ref = new DeferredReference<ViewportScissorParamPack>(GetCurrentOffset(records.back()));
        deferred_references.push_back(deferred_ref);
        return deferred_ref;

    }
    DeferredReference<ViewportScissorWithCountParamPack>* CommandRecord::SetViewportScissorWithCount(){
        BindCommand(records, CommandInstruction::Type::DS_ViewportScissorWithCount);
        auto deferred_ref = new DeferredReference<ViewportScissorWithCountParamPack>(GetCurrentOffset(records.back()));
        deferred_references.push_back(deferred_ref);
        return deferred_ref;
    }

    DeferredReference<Pipeline*>* CommandRecord::BindPipeline(){
        BindCommand(records, CommandInstruction::Type::BindPipeline);
        auto deferred_ref = new DeferredReference<Pipeline*>(GetCurrentOffset(records.back()));
        deferred_references.push_back(deferred_ref);
        return deferred_ref;
    }

    //the plan is to only have the single descriptor set, for all bindless textures
    //void BindDescriptor(VkDescriptorSet set);
    
    DeferredReference<GlobalPushConstant>* CommandRecord::Push(){
        //assert a pipeline is binded
        BindCommand(records, CommandInstruction::Type::PushConstant);
        auto deferred_ref = new DeferredReference<GlobalPushConstant>(GetCurrentOffset(records.back()));
        deferred_references.push_back(deferred_ref);
        return deferred_ref;
    }

    DeferredReference<RenderInfo>* CommandRecord::BeginRender(){
        //i can probably reduce the size of this, i dont know if it's necessary or not
        BindCommand(records, CommandInstruction::Type::BeginRender);
        auto deferred_ref = new DeferredReference<RenderInfo>(GetCurrentOffset(records.back()));
        deferred_references.push_back(deferred_ref);
        return deferred_ref;
    }
    void CommandRecord::EndRender(){
        BindCommand(records, CommandInstruction::Type::EndRender);
    }

    DeferredReference<PipelineBarrier>* CommandRecord::Barrier(){
        printf("this needs to be fixed\n");
        //BindCommand(records, CommandType::PipelineBarrier, sizeof(PipelineBarrier));
        auto deferred_ref = new DeferredReference<PipelineBarrier>(GetCurrentOffset(records.back()));
        deferred_references.push_back(deferred_ref);
        return deferred_ref;
    }

    DeferredReference<LabelParamPack>* CommandRecord::BeginLabel() noexcept{
        BindCommand(records, CommandInstruction::Type::BeginLabel);
        auto deferred_ref = new DeferredReference<LabelParamPack>(GetCurrentOffset(records.back()));
        deferred_references.push_back(deferred_ref);
        return deferred_ref;
    }
    void CommandRecord::EndLabel() noexcept{
        BindCommand(records, CommandInstruction::Type::EndLabel);
    }

    DeferredReference<VertexDrawParamPack>* CommandRecord::Draw(){
        BindCommand(records, CommandInstruction::Type::Draw);
        auto deferred_ref = new DeferredReference<VertexDrawParamPack>(GetCurrentOffset(records.back()));
        deferred_references.push_back(deferred_ref);
        return deferred_ref;
    }
    DeferredReference<IndexDrawParamPack>* CommandRecord::DrawIndexed(){
        BindCommand(records, CommandInstruction::Type::DrawIndexed);
        auto deferred_ref = new DeferredReference<IndexDrawParamPack>(GetCurrentOffset(records.back()));
        deferred_references.push_back(deferred_ref);
        return deferred_ref;
    }
    DeferredReference<DispatchParamPack>* CommandRecord::Dispatch(){
        BindCommand(records, CommandInstruction::Type::Dispatch);
        auto deferred_ref = new DeferredReference<DispatchParamPack>(GetCurrentOffset(records.back()));
        deferred_references.push_back(deferred_ref);
        return deferred_ref;
    }
} //namespace EWE