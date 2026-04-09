#include "EightWinds/Command/PackageRecord.h"

namespace EWE{
namespace Command{

    ParamPool PackageRecord::Compile() const{
        ParamPool ret{};
        {
            std::size_t total_param_size = 0;
            for(auto* pkg : packages){
                total_param_size += pkg->paramPool.params[0].Size();
            }
            for(uint8_t frame = 0; frame < max_frames_in_flight; frame++){
                ret.params[frame].Resize(total_param_size);
            }
        }

        std::size_t current_param_pos = 0;
        for(auto* pkg : packages){
            ret.instructions.insert(ret.instructions.end(), pkg->paramPool.instructions.begin(), pkg->paramPool.instructions.end());
            for(uint8_t frame = 0; frame < max_frames_in_flight; frame++){
                memcpy(ret.params[frame].memory + current_param_pos, pkg->paramPool.params[frame].memory, pkg->paramPool.params[0].Size());
            }
            current_param_pos += pkg->paramPool.params[0].Size();
        }

        return ret; //i dont think i care about the instruction pointers
    }

} //namespace Command
} //namespace EWE