#pragma once

#include "EightWinds/VulkanHeader.h"

namespace EWE{
    struct ShaderStage { //becomes ShaderStage
        enum Bits {
            Vertex = 0,
            TessControl,
            TessEval,
            Geometry,
            Task,
            Mesh,
            Fragment,
            Compute,

            COUNT
        };
        Bits value;
        constexpr ShaderStage() : value{ Bits::COUNT } {}
        constexpr ShaderStage(Bits v) : value{ v } {}
        constexpr operator Bits() const {
            return value;
        }

        constexpr ShaderStage(VkShaderStageFlagBits vkStage) {
            switch (vkStage) {
                case VK_SHADER_STAGE_VERTEX_BIT: value = Bits::Vertex; break;
                case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT: value = Bits::TessControl; break;
                case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: value = Bits::TessEval; break;
                case VK_SHADER_STAGE_GEOMETRY_BIT: value = Bits::Geometry; break;
                case VK_SHADER_STAGE_TASK_BIT_EXT: value = Bits::Task; break;
                case VK_SHADER_STAGE_MESH_BIT_EXT: value = Bits::Mesh; break;
                case VK_SHADER_STAGE_FRAGMENT_BIT: value = Bits::Fragment; break;
                case VK_SHADER_STAGE_COMPUTE_BIT: value = Bits::Compute; break;
                default: EWE_UNREACHABLE;value = Bits::Vertex;//error silencer
            }
            
        }
        constexpr operator VkShaderStageFlagBits() const {
            switch (value) {
                case Bits::Vertex:		return VK_SHADER_STAGE_VERTEX_BIT;
                case Bits::TessControl:	return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
                case Bits::TessEval:	return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
                case Bits::Geometry:    return VK_SHADER_STAGE_GEOMETRY_BIT;
                case Bits::Task:		return VK_SHADER_STAGE_TASK_BIT_EXT;
                case Bits::Mesh:		return VK_SHADER_STAGE_MESH_BIT_EXT;
                case Bits::Fragment:    return VK_SHADER_STAGE_FRAGMENT_BIT;
                case Bits::Compute:		return VK_SHADER_STAGE_COMPUTE_BIT;
                default: EWE_UNREACHABLE;
            }
            EWE_UNREACHABLE;
        }

        constexpr bool operator==(Bits bits) const {
            return value == bits;
        }
        constexpr bool operator==(ShaderStage const other) const {
            return value == other.value;
        }
        constexpr bool operator==(VkShaderStageFlagBits bits) const {
            return value == ShaderStage(bits);
        }
    };
} //namespace EWE