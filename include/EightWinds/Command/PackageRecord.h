#pragma once

#include "EightWinds/VulkanHeader.h"

#include "EightWinds/Command/InstructionPointer.h"
#include "EightWinds/Command/Instruction.h"
#include "EightWInds/Command/InstructionPackage.h"

//#include "EightWinds/Data/RuntimeArray.h"
#include "InstructionPackage.h"

#include <string_view>
#include <vector>

namespace EWE{
namespace Command{

        /*
            need a package of packages
            it looks like record is the best fit for that

            option 1
                only allow record to contain packages, easiest
            option 2
                allow record to contain packages and instructions - least lossy
            
            if i only allow packages, users won't be able to create any random behavior
                they'll be force to work within the bounds of what i support
            CURRENTLY, that's probably fine

            im making a packagerecord just so i dont have to deal with losing record until i finish
        */
    struct PackageRecord{
        std::string name{};
        std::vector<InstructionPackage*> packages;

        [[nodiscard]] PackageRecord() = default;
        [[nodiscard]] PackageRecord(std::filesystem::path const& file_location);

        PackageRecord(PackageRecord const&) = delete;
        PackageRecord& operator=(PackageRecord const&) = delete;
        PackageRecord(PackageRecord&&) = delete;
        PackageRecord& operator=(PackageRecord&&) = delete;
    };
} //namespace Command
} //namespace EWE