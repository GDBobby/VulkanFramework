#pragma once

#include "EightWinds/VulkanHeader.h"

#include "EightWinds/Command/InstructionPointer.h"
#include "EightWinds/Command/Instruction.h"
#include "EightWinds/Command/InstructionPackage.h"

//#include "EightWinds/Data/RuntimeArray.h"
#include "InstructionPackage.h"

#include <vector>
#include <filesystem>

namespace EWE{
    struct Queue;

namespace Command{
    struct PackageRecord{
        std::filesystem::path name{};
        Queue& queue;
        std::vector<InstructionPackage*> packages;

        [[nodiscard]] explicit PackageRecord(std::filesystem::path const& name, Queue& queue);

        PackageRecord(PackageRecord const&) = delete;
        PackageRecord& operator=(PackageRecord const&) = delete;
        PackageRecord(PackageRecord&&) = delete;
        PackageRecord& operator=(PackageRecord&&) = delete;

        ParamPool Compile() const;
    };
} //namespace Command
} //namespace EWE