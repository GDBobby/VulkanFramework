#include "EightWinds/Command/Record.h"

namespace EWE{
    
    void BindCommand(std::vector<CommandInstruction>& records, CommandInstruction::Type cmdType){
        std::size_t paramOffset = 0;
        if(records.size() > 0){
            paramOffset = records.back().paramOffset + CommandInstruction::GetParamSize(records.back().type);
        }
        records.push_back(CommandInstruction{cmdType, paramOffset});

    }
    void BindCommand(std::vector<CommandInstruction>& records, CommandInstruction::Type cmdType, std::size_t paramSize){
        std::size_t paramOffset = 0;
        if(records.size() > 0){
            paramOffset = records.back().paramOffset + CommandInstruction::GetParamSize(records.back().type);
        }
        records.push_back(CommandInstruction{cmdType, paramOffset, paramSize});

    }
    

    void CommandRecord::BindPipeline(){
        BindCommand(records, CommandInstruction::Type::BindPipeline, sizeof(Pipeline*));
    }

    //i think i might need more data here
    //void BindDescriptor(VkDescriptorSet set);
    
    void CommandRecord::Push(void* push, std::size_t pushSize){
        //assert a pipeline is binded
        BindCommand(records, CommandInstruction::Type::PushConstants, pushSize);
    }

    void CommandRecord::BeginRender(){
        //i can probably reduce the size of this, i dont know if it's necessary or not
        BindCommand(records, CommandInstruction::Type::BeginRender);
    }
    void CommandRecord::EndRender(){
        BindCommand(records, CommandInstruction::Type::EndRender, 0);
    }

    void CommandRecord::Barrier(){
        printf("this needs to be fixed\n");
        //BindCommand(records, CommandType::PipelineBarrier, sizeof(PipelineBarrier));
    }

    void CommandRecord::BeginLabel(const char* name, float red, float green, float blue) noexcept{
        BindCommand(records, CommandInstruction::Type::BeginLabel, sizeof(const char*) + sizeof(float) * 3);
    }
    void CommandRecord::EndLabel() noexcept{
        BindCommand(records, CommandInstruction::Type::EndLabel, 0);
    }
    void CommandRecord::SetDynamicState(VkDynamicState dynState){
        printf("this needs to be split into each unique dynamic state\n");
        BindCommand(records, CommandInstruction::Type::SetDynamicState, sizeof(VkDynamicState) + sizeof(void*));
    }

    void CommandRecord::Draw(){
        BindCommand(records, CommandInstruction::Type::Draw, 0);
    }
    void CommandRecord::DrawIndexed(){
        BindCommand(records, CommandInstruction::Type::DrawIndexed, 0);
    }
    void CommandRecord::Dispatch(){
        BindCommand(records, CommandInstruction::Type::Dispatch, sizeof(uint32_t) * 3);
    }
        



} //namespace EWE