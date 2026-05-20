#pragma once

#include "EightWinds/Command/InstructionPackage.h"
#include "EightWinds/Command/PackageRecord.h"


namespace EWE{
    struct ParamPointerChain{
        Command::PackageRecord* base;
        std::size_t package_iter;
        std::vector<std::size_t> pointer_into; //this will be stable for each frame in flight into a parampool
    
        bool operator==(ParamPointerChain const& other) const;
    };
} //namespace EWE