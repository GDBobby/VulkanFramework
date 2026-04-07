#include "EightWinds/Command/ParamPool.h"
#include "EightWinds/VulkanHeader.h"

namespace EWE{
namespace Command{

    void ParamPool::ReadjustOffsets(PerFlight<std::size_t> previous_addr, PerFlight<std::size_t> next_addr){
        for(uint8_t frame = 0; frame < max_frames_in_flight; frame++){
            for(std::size_t i = 0; i < param_data.size(); i++){
                const std::size_t offset = param_data[i].data[frame] - previous_addr[frame];
                param_data[i].data[frame] = next_addr[frame] + offset;
            }
        }
    }

    void ParamPool::PushBack(Inst::Type itype){
        const std::size_t added_inst_size = Instruction::GetParamSize(itype);
        instructions.push_back(itype);
        if(added_inst_size > 0){
            auto& inst_back = param_data.emplace_back();

            const PerFlight<std::size_t> previous_memory_addresses{reinterpret_cast<std::size_t>(params[0].memory), reinterpret_cast<std::size_t>(params[1].memory)};
            const std::size_t param_pool_size = params[0].Size();
            for(uint8_t frame = 0; frame < max_frames_in_flight; frame++) {
                //temporarily, this will be a dangling ptr, and point outside of accounted memory bounds
                inst_back.data[frame] = reinterpret_cast<std::size_t>(&params[frame].memory[param_pool_size]);
                inst_back.adjusted = true;
                HeapBlock<uint8_t> temp{param_pool_size + added_inst_size};
                memcpy(temp.memory, params[frame].memory, param_pool_size);
                params[frame].Clear();
                params[frame].memory = temp.memory;
                params[frame].size = temp.size;
                temp.memory = nullptr;
                temp.size = 0;
            }
            const PerFlight<std::size_t> current_memory_addresses{reinterpret_cast<std::size_t>(params[0].memory), reinterpret_cast<std::size_t>(params[1].memory)};
            ReadjustOffsets(previous_memory_addresses, current_memory_addresses);
        }
    }

    std::size_t ParamPool::GetPackIndex(std::size_t inst_index) const{
        std::size_t current_pack_index = 0;
        for(std::size_t i = 0; i < inst_index; i++){
            current_pack_index += Instruction::GetParamSize(instructions[inst_index]) != 0;
        }
        return current_pack_index;
    }
    std::size_t ParamPool::GetParamOffset(std::size_t inst_index) const{
        const std::size_t pack_index = GetPackIndex(inst_index);
        if(pack_index < param_data.size()){
            return param_data[pack_index].data[0] - param_data[0].data[0];
        }
        return param_data.size();
    }

    void ParamPool::PopBack(){
        Erase(instructions.size() - 1);
        /*
        if(instructions.size() == 0){
            Logger::Print<Logger::Warning>("attempting to pop an empty param pool\n");
            return;
        }
        const std::size_t back_param_size = Instruction::GetParamSize(instructions.back());
        if(back_param_size == 0){
            return;
        }
        auto& back_param = param_data.back();
        const std::size_t back_param_offset = back_param.data[0] - param_data[0].data[0];

        for(uint8_t frame = 0; frame < max_frames_in_flight; frame++) {
            HeapBlock<uint8_t> temp{param_pool_size + added_inst_size};
            memcpy(temp.memory, params[frame].memory, removed_pack_start);
            memcpy(temp.memory + removed_pack_start, params[frame].memory + removed_pack_start + removed_inst_size, param_pool_size - removed_inst_size);
            params[frame].Clear();
            params[frame].memory = temp.memory;
            params[frame].size = temp.size;
            temp.memory = nullptr;
            temp.size = 0;
        }
        */
    }

    void ParamPool::Erase(std::size_t index){
        if(index >= instructions.size() || instructions.size() == 0){
            Logger::Print<Logger::Warning>("attempting to erase out of bounds\n");
            return;
        }

        const std::size_t removed_inst_size = Instruction::GetParamSize(instructions[index]);
        if(removed_inst_size > 0){
            const std::size_t starting_size = params[0].Size();
            const std::size_t pack_index = GetPackIndex(index);
            const std::size_t removed_addr = param_data[pack_index].data[0];
            const std::size_t starting_addr = param_data[0].data[0];
            const std::size_t removed_pack_start = removed_addr - starting_addr;

            const PerFlight<std::size_t> previous_memory_addresses{reinterpret_cast<std::size_t>(params[0].memory), reinterpret_cast<std::size_t>(params[1].memory)};
            param_data.erase(param_data.begin() + pack_index);

            for(uint8_t frame = 0; frame < max_frames_in_flight; frame++) {
                HeapBlock<uint8_t> temp{starting_size - removed_inst_size};
                memcpy(temp.memory, params[frame].memory, removed_pack_start);
                memcpy(temp.memory + removed_pack_start, params[frame].memory + removed_pack_start + removed_inst_size, starting_size - removed_pack_start - removed_inst_size);
                params[frame].Clear();
                params[frame].memory = temp.memory;
                params[frame].size = temp.size;
                temp.memory = nullptr;
                temp.size = 0;
                for(std::size_t i = pack_index; i < param_data.size(); i++){
                    param_data[i].data[frame] -= removed_inst_size;
                }
            }
            
            const PerFlight<std::size_t> current_memory_addresses{reinterpret_cast<std::size_t>(params[0].memory), reinterpret_cast<std::size_t>(params[1].memory)};
            ReadjustOffsets(previous_memory_addresses, current_memory_addresses);
        }
        instructions.erase(instructions.begin() + index);

            

    }
    
    void ParamPool::ShrinkToSize(std::size_t index){
        if(index == (instructions.size() - 1)){
            PopBack();
            return;
        }

        std::size_t first_erasure = param_data.size();
        for(std::size_t i = index; i < instructions.size(); i++){
            if(Instruction::GetParamSize(instructions[i]) > 0){
                first_erasure = i;
                break;
            }
        }
        if(first_erasure < param_data.size()){
            const std::size_t shrunk_size = param_data[first_erasure].data[0] - param_data[0].data[0];
            //const std::size_t starting_size = params[0].Size();
            instructions.erase(instructions.begin() + index, instructions.end());
            param_data.erase(param_data.begin() + first_erasure, param_data.end());

            const PerFlight<std::size_t> previous_memory_addresses{reinterpret_cast<std::size_t>(params[0].memory), reinterpret_cast<std::size_t>(params[1].memory)};
            for(uint8_t frame = 0; frame < max_frames_in_flight; frame++){

                HeapBlock<uint8_t> temp{shrunk_size};
                memcpy(temp.memory, params[frame].memory, shrunk_size);
                params[frame].Clear();
                params[frame].memory = temp.memory;
                params[frame].size = temp.size;
                temp.memory = nullptr;
                temp.size = 0;
            }

            const PerFlight<std::size_t> current_memory_addresses{reinterpret_cast<std::size_t>(params[0].memory), reinterpret_cast<std::size_t>(params[1].memory)};
            ReadjustOffsets(previous_memory_addresses, current_memory_addresses);
        }
    }
    void ParamPool::Insert(std::size_t index, Inst::Type itype){
        EWE_ASSERT(index <= instructions.size());
        std::size_t added_inst_size = Instruction::GetParamSize(itype);
        instructions.insert(instructions.begin() + index, itype);
        if(added_inst_size > 0){
            std::size_t param_pool_size = params[0].Size();
            const std::size_t param_offset = GetParamOffset(index);
            const auto pack_index = GetPackIndex(index);
            param_data.insert(param_data.begin() + pack_index, InstructionPointerAdjuster{});
            auto& inst_ptr = param_data[pack_index];
            
            const PerFlight<std::size_t> previous_memory_addresses{reinterpret_cast<std::size_t>(params[0].memory), reinterpret_cast<std::size_t>(params[1].memory)};
            
            for(uint8_t frame = 0; frame < max_frames_in_flight; frame++) {
                HeapBlock<uint8_t> temp{param_pool_size + added_inst_size};
                memcpy(temp.memory, params[frame].memory, param_offset);
                memcpy(temp.memory + param_offset + added_inst_size, params[frame].memory + param_offset, param_pool_size - param_offset);
                params[frame].Clear();
                params[frame].memory = temp.memory;
                params[frame].size = temp.size;
                temp.memory = nullptr;
                temp.size = 0;
                inst_ptr.data[frame] = reinterpret_cast<std::size_t>(&params[frame].memory[param_offset]);

                inst_ptr.adjusted = true;
            }

            const PerFlight<std::size_t> current_memory_addresses{reinterpret_cast<std::size_t>(params[0].memory), reinterpret_cast<std::size_t>(params[1].memory)};
            ReadjustOffsets(previous_memory_addresses, current_memory_addresses);
        }
    }
} //namespce Command
} //namespace EWE