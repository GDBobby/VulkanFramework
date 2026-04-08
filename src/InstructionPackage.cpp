#include "EightWinds/Command/InstructionPackage.h"

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

    InstructionPackage::InstructionPackage()
    : type{InstructionPackage::Base},
    paramPool{}
    {
    }
    InstructionPackage::InstructionPackage(Type _type)
        : type{_type},
        paramPool{}
    {

    }

} //namespace Command
} //namespace EWE