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
        void PushBack(Inst::Type itype);
        void Insert(std::size_t index, Inst::Type itype);

        void ReadjustOffsets(PerFlight<std::size_t> previous_addr, PerFlight<std::size_t> next_addr);
        
        template<Inst::Type IType>
        requires (std::meta::is_complete_type(^^ParamPack<IType>))
        void PushBack(ParamPack<IType> const& pp){
            PushBack(IType);
            for(uint8_t frame = 0; frame < max_frames_in_flight; frame++){
                memcpy(param_data.back().data[frame], &pp, sizeof(ParamPack<IType>));
            }
        }
        template<Inst::Type IType>
        requires (std::meta::is_complete_type(^^ParamPack<IType>))
        void PushBack(PerFlight<ParamPack<IType>*> const& pp){
            PushBack(IType);
            for(uint8_t frame = 0; frame < max_frames_in_flight; frame++){
                memcpy(param_data.back().data[frame], pp[frame], sizeof(ParamPack<IType>));
            }
        }

        template<Inst::Type IType>
        requires (std::meta::is_complete_type(^^ParamPack<IType>))
        void Insert(std::size_t index, ParamPack<IType> const& pp){
            Insert(index, IType);
            const std::size_t pack_index = GetPackIndex(index);
            for(uint8_t frame = 0; frame < max_frames_in_flight; frame++){
                memcpy(param_data[pack_index].data[frame], &pp, sizeof(ParamPack<IType>));
            }
        }
        template<Inst::Type IType>
        requires (std::meta::is_complete_type(^^ParamPack<IType>))
        void PushBack(std::size_t index, PerFlight<ParamPack<IType>*> const& pp){
            Insert(index, IType);
            const std::size_t pack_index = GetPackIndex(index);
            for(uint8_t frame = 0; frame < max_frames_in_flight; frame++){
                memcpy(param_data[pack_index].data[frame], pp[frame], sizeof(ParamPack<IType>));
            }
        }

    };

} //namepsace Command
} //namepsace EWE