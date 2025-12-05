#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/LogicalDevice.h"
#include "EightWinds/Backend/Descriptor/SetLayout.h"

#include <vector>
#include <mutex> //factory

namespace EWE {
	struct Shader {
        LogicalDevice& logicalDevice;

        struct Stage { //becomes Shader::Stage
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
            constexpr Stage() : value{ Bits::COUNT } {}
            constexpr Stage(Bits v) : value{ v } {}
            constexpr operator Bits() const {
                return value;
            }

            constexpr Stage(VkShaderStageFlagBits vkStage) {
                switch (vkStage) {
                    case VK_SHADER_STAGE_VERTEX_BIT: value = Bits::Vertex; break;
                    case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT: value = Bits::TessControl; break;
                    case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: value = Bits::TessEval; break;
                    case VK_SHADER_STAGE_GEOMETRY_BIT: value = Bits::Geometry; break;
                    case VK_SHADER_STAGE_TASK_BIT_EXT: value = Bits::Task; break;
                    case VK_SHADER_STAGE_MESH_BIT_EXT: value = Bits::Mesh; break;
                    case VK_SHADER_STAGE_FRAGMENT_BIT: value = Bits::Fragment; break;
                    case VK_SHADER_STAGE_COMPUTE_BIT: value = Bits::Compute; break;
                }
                //have fun debugging this when it doesnt work.
                //EWE_UNREACHABLE;
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
                }
                //have fun debugging this when it doesnt work
                //EWE_UNREACHABLE;
            }

            constexpr bool operator==(Bits bits) const {
                return value == bits;
            }
            constexpr bool operator==(Stage const other) const {
                return value == other.value;
            }
            constexpr bool operator==(VkShaderStageFlagBits bits) const {
                return value == Stage(bits);
            }
        };

		enum FundamentalType {
			ST_INT,
			ST_UINT,
			ST_BOOL,
			ST_FLOAT,
			ST_DOUBLE,

			ST_COUNT
		};

		struct SpecializationEntry {
#if PIPELINE_HOT_RELOAD
			std::string name{};
#endif
			FundamentalType type;
			uint32_t constantID;
			uint8_t elementCount = 1;
			char value[64]; //64 being the size of a 4x4 matrix, the largest size im supporting rn. i dont even know if thats a thing for spec constants
		};

		struct VkSpecInfo_RAII {
			VkSpecInfo_RAII() {}
			VkSpecInfo_RAII(std::vector<SpecializationEntry> const& specEntries);
			VkSpecInfo_RAII(VkSpecInfo_RAII const& copy);
			VkSpecInfo_RAII& operator=(VkSpecInfo_RAII const& copy) = delete;
			VkSpecInfo_RAII(VkSpecInfo_RAII&& move) noexcept;
			VkSpecInfo_RAII& operator=(VkSpecInfo_RAII&& move) = delete;
			~VkSpecInfo_RAII();
			std::vector<VkSpecializationMapEntry> mapEntries{};
			VkSpecializationInfo specInfo{};
			uint64_t memPtr = 0;
		};


		std::string filepath{}; 
		VkPipelineShaderStageCreateInfo shaderStageCreateInfo;

		Backend::Descriptor::LayoutPack descriptorSets;

        //VkPipelineVertexInputStateCreateInfo
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributes{};
		VkPushConstantRange pushRange{};

		std::vector<SpecializationEntry> defaultSpecConstants{};

        struct ShaderStruct {
            std::string name;
            std::size_t size;
            struct Member {
                std::string name;
                FundamentalType type;
                uint32_t offset;
            };
            std::vector<Member> members{};
        };
        std::vector<ShaderStruct> structData{};

		explicit Shader(LogicalDevice& logicalDevice, std::string_view fileLocation);
		Shader(LogicalDevice& logicalDevice, std::string_view fileLocation, const std::size_t dataSize, const void* data);
		Shader(LogicalDevice& logicalDevice);
		~Shader();

		bool ValidateVertexInputAttributes(std::vector<VkVertexInputAttributeDescription> const& cpu_side) const;
		VkShaderModule GetVkShader() const {
			return shaderStageCreateInfo.module;
		}
        
#if PIPELINE_HOT_RELOAD
        void HotReload();
#endif

	//protected:
		void CompileModule(const std::size_t dataSize, const void* data);
		void ReadReflection(const std::size_t dataSize, const void* data);
	};
}