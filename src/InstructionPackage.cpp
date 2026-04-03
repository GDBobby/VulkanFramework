#include "EightWinds/Command/InstructionPackage.h"
#include "EightWinds/Command/RasterInstructionPackage.h"

#include <fstream>

namespace EWE{
namespace Command{


    struct InstPackageFileHeader{
        //just for reference
        std::size_t version;
        uint8_t package_type;
        std::size_t instruction_count;
        std::size_t param_data_size;

        //instruction_data
        //param_data
        //any other data, potentially even given more descriptors
    };

} //namespace Command
} //namespace EWE