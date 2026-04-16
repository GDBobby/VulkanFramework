#pragma once

#include "EightWinds/Backend/ShaderVariable.h"

#include "EightWinds/Data/RuntimeArray.h"

#include <cstdint>

namespace EWE{

    /*
    struct SpecializationEntry {
#if PIPELINE_HOT_RELOAD
        std::string name{};
#endif
        ShaderVariable::Type type;
        uint32_t constantID;
        uint8_t elementCount = 1;
        char value[64]; //64 being the size of a 4x4 matrix, the largest size im supporting rn. i dont even know if thats a thing for spec constants
    };

    struct VkSpecInfo_RAII {
        VkSpecInfo_RAII(RuntimeArray<SpecializationEntry> const& specEntries);
        VkSpecInfo_RAII(VkSpecInfo_RAII const& copy);
        VkSpecInfo_RAII& operator=(VkSpecInfo_RAII const& copy) = delete;
        VkSpecInfo_RAII(VkSpecInfo_RAII&& move) noexcept;
        VkSpecInfo_RAII& operator=(VkSpecInfo_RAII&& move) = delete;
        ~VkSpecInfo_RAII();

        RuntimeArray<VkSpecializationMapEntry> mapEntries;
        VkSpecializationInfo specInfo{};
        uint64_t memPtr = 0;
    };
    */
}