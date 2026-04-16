#pragma once

#include "EightWinds/VulkanHeader.h"

#include "EightWinds/Data/RuntimeArray.h"

#include <string>
#include <cstdint>

namespace EWE{
    struct ShaderVariable{
        enum class Type{
            Unknown,//error?
            Void,

            //Integral,
                Bool, //is this integral? or is this a literal assembly flag
                Int8, //sbyte
                UInt8, //ubyte
                Int16,
                UInt16,
                Int32,
                UInt32,
                Int64,
                UInt64,

            //FP,
                Float16,
                Float32,
                Float64,

            Image,
            Sampler,
            Struct,
            SampledImage,

            AtomicCounter,

        };

        std::string name;

        Type baseType = Type::Unknown;
        uint32_t width; //what is width
        uint32_t vecsize;
        uint32_t columns;

        //if this variable is not an array, this will be equal to {1}
        //if it is an array of 2 arrays, each with a depth of 3, it will be {3}, {3}
        RuntimeArray<uint32_t> array_lengths{1, 1};
        
        bool is_literal = false;
        uint32_t size;
        uint32_t offset = 0; //if its a member of a struct, or a push constant
    
        uint32_t pointer_depth;
        bool pointer;
        bool forward_pointer;

        bool written_to = false;

        RuntimeArray<ShaderVariable*> members{0};
    };
}