#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/LogicalDevice.h"

#include "EightWinds/Backend/ShaderStage.h"
#include "EightWinds/Backend/ShaderVariable.h"
#include "EightWinds/Backend/ShaderSpecialization.h"

#include "EightWinds/Data/RuntimeArray.h"
#include "EightWinds/Data/Hive.h"

#include <vector>

namespace EWE {

    struct ShaderMeta{
        static constexpr uint64_t file_version = 0;
        RuntimeArray<bool> buffer_written_to;
        RuntimeArray<bool> texture_written_to;

        [[nodiscard]] ShaderMeta() 
        : buffer_written_to{0}, 
            texture_written_to{0} 
        {}

        void WriteToFile(std::filesystem::path const& path);
        bool ReadFromFile(std::filesystem::path const& path);
    };


    struct PushConstant{
        uint32_t offset;
        uint32_t size;

        struct Member{
            std::string name;
            uint32_t offset;
            uint32_t size;
            enum Type{
                Buffer,
                Texture
            };
            Type type;
        };
        std::vector<Member> buffers;
        std::vector<Member> textures;

        [[nodiscard]] PushConstant();

        operator VkPushConstantRange() const{
            return VkPushConstantRange{
                //i dont think tghis has a performance impact. if it does, i'll need to remove it 
                .stageFlags = VK_SHADER_STAGE_ALL,
                .offset = offset,
                .size = size
            };
        }
    };

	struct Shader {
        LogicalDevice& logicalDevice;

		std::filesystem::path name{}; 

		VkPipelineShaderStageCreateInfo shaderStageCreateInfo;

        PushConstant pushRange;
        ShaderMeta meta;

		//RuntimeArray<SpecializationEntry> defaultSpecConstants{0};

		[[nodiscard]] explicit Shader(LogicalDevice& logicalDevice, std::filesystem::path const& fileLocation);
        [[nodiscard]] explicit Shader(LogicalDevice& logicalDevice, std::filesystem::path const& fileLocation, const std::size_t dataSize, const void* data);
        [[nodiscard]] explicit Shader(LogicalDevice& logicalDevice);
		~Shader();
        Shader(Shader const& copySrc) = delete;
        Shader(Shader&& moveSrc) = delete;
        Shader& operator=(Shader const& copySrc) = delete;
        Shader& operator=(Shader&& moveSrc) = delete;

        bool has_bindless_textures = false;

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
        void SetDebugName(std::string_view name);

	//protected:
		void CompileModule(const std::size_t dataSize, const void* data);
		void ReadReflection(const std::size_t dataSize, const void* data);

	    struct BufferReference {
	    	std::string name;
	    	uint32_t size;
            uint32_t alignment;
	    };
		std::vector<BufferReference> bufferReferences;

        //Hive<ShaderVariable> variables;
        //the int is spirv_cross id
        //std::unordered_map<int, ShaderVariable*> existing_variables; 
	};
}