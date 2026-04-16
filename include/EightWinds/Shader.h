#pragma once

#include "EightWinds/GlobalPushConstant.h"
#include "EightWinds/VulkanHeader.h"
#include "EightWinds/LogicalDevice.h"
#include "EightWinds/Backend/Descriptor/SetLayout.h"

#include "EightWinds/Backend/ShaderStage.h"
#include "EightWinds/Backend/ShaderVariable.h"
#include "EightWinds/Backend/ShaderSpecialization.h"

#include "EightWinds/Data/RuntimeArray.h"
#include "EightWinds/Data/Hive.h"

#include <vector>

namespace EWE {
	struct Shader {
        LogicalDevice& logicalDevice;

		std::filesystem::path filepath{}; 

		VkPipelineShaderStageCreateInfo shaderStageCreateInfo;
		Backend::Descriptor::LayoutPack descriptorSets;

        struct PushConstant{
            uint32_t offset;
            uint32_t size;
            struct BufferAddress{
                bool writtenTo = false;
                bool inUse = false;
                ShaderVariable* variable;
            };
            struct TextureIndex{
                bool writtenTo = false;
                bool inUse = false;
                ShaderVariable* variable;
            };
            std::array<BufferAddress, GlobalPushConstant_Raw::buffer_count> buffers;
            std::array<TextureIndex, GlobalPushConstant_Raw::texture_count> textures;

            operator VkPushConstantRange() const{
                return VkPushConstantRange{
                    .stageFlags = VK_SHADER_STAGE_ALL,
                    .offset = offset,
                    .size = size
                };
            }
        };
        PushConstant pushRange;

		//RuntimeArray<SpecializationEntry> defaultSpecConstants{0};

		[[nodiscard]] explicit Shader(LogicalDevice& logicalDevice, std::filesystem::path const& fileLocation);
        [[nodiscard]] explicit Shader(LogicalDevice& logicalDevice, std::filesystem::path const& fileLocation, const std::size_t dataSize, const void* data);
        [[nodiscard]] explicit Shader(LogicalDevice& logicalDevice);
		~Shader();
        Shader(Shader const& copySrc) = delete;
        Shader(Shader&& moveSrc) = delete;
        Shader& operator=(Shader const& copySrc) = delete;
        Shader& operator=(Shader&& moveSrc) = delete;

		//bool ValidateVertexInputAttributes(std::vector<VkVertexInputAttributeDescription> const& cpu_side) const;
		VkShaderModule GetVkShader() const {
			return shaderStageCreateInfo.module;
		}
        ShaderStage GetStage() const{
            return ShaderStage{shaderStageCreateInfo.stage};
        }
        
#if PIPELINE_HOT_RELOAD
        void HotReload();
#endif
#if EWE_DEBUG_NAMING
        void SetDebugName(std::string_view name);
#endif

	//protected:
		void CompileModule(const std::size_t dataSize, const void* data);
		void ReadReflection(const std::size_t dataSize, const void* data);

        Hive<ShaderVariable> variables;
        //the int is spirv_cross id
        std::unordered_map<int, ShaderVariable*> existing_variables; 
	};
}