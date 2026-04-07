#pragma once
#include "EightWinds/Command/InstructionType.h"
#include "EightWinds/Data/HeapBlock.h"
#include "EightWinds/Command/InstructionPointer.h"
#include "EightWinds/Command/ParamPacks.h"
#include "EightWinds/Command/Instruction.h"

#include <string.h> //memcpy

namespace EWE{
namespace Command{
    //can this be template generated? would that be worth the effort?
    struct ParamPool{
        /*
            * using a heap block instead of a stack block allows this to
                be easily moved, copied, and resized
            * might be better to use something that can handle a disparity between size and capacity
                it would mean less moves
        */
        PerFlight<HeapBlock<uint8_t>> params; 
        std::vector<Inst::Type> instructions;

        //instruction pointer adjuster being moved or copied is acceptable here, the underlying data has a reliable state
        //the inst pointers are 1:1 with instructions. if no params, then nullptr
        std::vector<InstructionPointerAdjuster> param_data;

        std::size_t GetPackIndex(std::size_t inst_index) const;
        std::size_t GetParamOffset(std::size_t inst_index) const;
        void Erase(std::size_t index);
        void ShrinkToSize(std::size_t index);

        void PopBack();
        void PushBack_Fresh(Inst::Type itype);
        void Insert_Fresh(std::size_t index, Inst::Type itype);

        void ReadjustOffsets(PerFlight<std::size_t> previous_addr, PerFlight<std::size_t> next_addr);
        
        template<Inst::Type IType>
        void PushBack_Existing(ParamPack<IType> const& pp){
            const std::size_t added_inst_size = Instruction::GetParamSize(IType);
            //the pack iwll be copied
            PushBack_Fresh(IType);
            if(added_inst_size > 0){
                auto& inst_back = param_data.back();
                for(uint8_t frame = 0; frame < max_frames_in_flight; frame++){
                    memcpy(reinterpret_cast<void*>(inst_back.data[frame]), &pp, sizeof(ParamPack<IType>));
                }
            }
        }
        template<Inst::Type IType>
        void PushBack_Existing(PerFlight<ParamPack<IType>*> const& pp){
            const std::size_t added_inst_size = Instruction::GetParamSize(IType);
            //the pack iwll be copied
            PushBack_Fresh(IType);
            if(added_inst_size > 0){
                auto& inst_back = param_data.back();
                for(uint8_t frame = 0; frame < max_frames_in_flight; frame++){
                    memcpy(reinterpret_cast<void*>(inst_back.data[frame]), pp[frame], sizeof(ParamPack<IType>));
                }
            }
        }

        template<Inst::Type IType>
        void Insert_Existing(std::size_t index, ParamPack<IType> const& pp){
            EWE_ASSERT(index <= instructions.size());
            std::size_t added_inst_size = Instruction::GetParamSize(IType);
            instructions.insert(instructions.begin() + index, IType);
            //auto* inst_ptr = record.Add(itype);
            if(added_inst_size > 0){
                std::size_t param_pool_size = params[0].Size();
                const std::size_t param_offset = GetParamOffset(index);
                auto* inst_ptr = param_data.insert(param_data.begin() + GetPackIndex(index), InstructionPointerAdjuster{});
                
                for(uint8_t frame = 0; frame < max_frames_in_flight; frame++) {
                    HeapBlock<uint8_t> temp{param_pool_size + added_inst_size};
                    memcpy(temp.memory, params[frame].memory, param_offset);
                    memcpy(temp.memory + param_offset + added_inst_size, params[frame].memory + param_offset, param_pool_size - param_offset);
                    params[frame].Clear();
                    params[frame].memory = temp.memory;
                    params[frame].size = temp.size;
                    temp.memory = nullptr;
                    temp.size = 0;
                    inst_ptr->data[frame] = reinterpret_cast<std::size_t>(&params[frame].memory[param_offset]);
                    inst_ptr->adjusted = true;
                    memcpy(&params[frame].memory[param_offset], &pp, sizeof(ParamPack<IType>));
                }
            }
        }
        template<Inst::Type IType>
        void PushBack_Existing(std::size_t index, PerFlight<ParamPack<IType>*> const& pp){
            EWE_ASSERT(index <= instructions.size());
            const std::size_t added_inst_size = Instruction::GetParamSize(IType);
            instructions.insert(instructions.begin() + index, IType);
            //auto* inst_ptr = record.Add(itype);
            if(added_inst_size > 0){
                std::size_t param_pool_size = params[0].Size();
                const std::size_t param_offset = GetParamOffset(index);
                auto* inst_ptr = param_data.insert(param_data.begin() + GetPackIndex(index), InstructionPointerAdjuster{});
                
                for(uint8_t frame = 0; frame < max_frames_in_flight; frame++) {
                    HeapBlock<uint8_t> temp{param_pool_size + added_inst_size};
                    memcpy(temp.memory, params[frame].memory, param_offset);
                    memcpy(temp.memory + param_offset + added_inst_size, params[frame].memory + param_offset, param_pool_size - param_offset);
                    params[frame].Clear();
                    params[frame].memory = temp.memory;
                    params[frame].size = temp.size;
                    temp.memory = nullptr;
                    temp.size = 0;
                    inst_ptr->data[frame] = reinterpret_cast<std::size_t>(&params[frame].memory[param_offset]);
                    inst_ptr->adjusted = true;
                    memcpy(&params[frame].memory[param_offset], pp[frame], sizeof(ParamPack<IType>));
                }
            }
        }

    };

} //namepsace Command
} //namepsace EWE