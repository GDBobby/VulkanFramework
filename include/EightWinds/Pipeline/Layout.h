#pragma once
#include "EightWinds/VulkanHeader.h"
#include "EightWinds/Shader.h"

#include "EightWinds/Data/KeyValueContainer.h"

namespace EWE {
	enum class PipelineType {
		Vertex, //combination of vertex, fragment - optional geom/tess
		Compute,
		Mesh, //task mesh
		MeshWithMeshDisabled,
		Raytracing, //any RT combo
		Scheduler, //graph/scheduling - distinct? idk

		COUNT
	};

	constexpr VkPipelineBindPoint BindPointFromType(PipelineType pt) {
		switch (pt) {
		case PipelineType::Vertex: return VK_PIPELINE_BIND_POINT_GRAPHICS;
		case PipelineType::Compute: return VK_PIPELINE_BIND_POINT_COMPUTE;
		case PipelineType::Mesh: return VK_PIPELINE_BIND_POINT_GRAPHICS;
		case PipelineType::MeshWithMeshDisabled: return VK_PIPELINE_BIND_POINT_GRAPHICS; //this is a bit more ambiguous
		case PipelineType::Raytracing: return VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
			//Scheduler, //graph/scheduling - distinct? idk
		}
		EWE_UNREACHABLE;
	}
	/* template specializing pipelines
	template<PipelineType>
	struct PipelineTraits;

	template<>
	struct PipelineTraits<PipelineType::Vertex> {
		static constexpr std::array stages{
			ShaderStage::Vertex,
			ShaderStage::Fragment,
			ShaderStage::Geometry,
			ShaderStage::TessControl,
			ShaderStage::TessEval,
		};
	};
	template<>
	struct PipelineTraits<PipelineType::Mesh> {
		static constexpr std::array stages{
			ShaderStage::Task,
			ShaderStage::Mesh,
			ShaderStage::Vertex
		};
	};

	template<>
	struct PipelineTraits<PipelineType::Compute> {
		static constexpr std::array stages{
			ShaderStage::Compute,
		};
		static constexpr std::string_view GetStageName(uint8_t index) {
			return magic_enum::enum_name(stages[index]);
		}
	};

	template<PipelineType>
	static constexpr uint8_t GetPipeLayoutStageIndex(ShaderStage stage) {
		for (uint8_t i = 0; i < PipelineTraits<PipelineType>::stages.size(); i++) {
			if (stage == PipelineTraits<PipelineType>::stages[i]) {
				return i;
			}
		}
		EWE_UNREACHABLE;
		return 0; //error silencing
	}
	*/


	struct PipeLayout {
		LogicalDevice& logicalDevice;
		VkPipelineLayout vkLayout;

		//using PipeTraits = PipelineTraits<PipelineType>;
		//i dont like the array much, i might do a KeyValuePair or something
		std::array<Shader*, Shader::Stage::COUNT> shaders;

		Descriptor::LayoutPack descriptorSets;
		std::vector<VkPushConstantRange> pushConstantRanges{};
		PipelineType pipelineType;

		std::vector<VkPipelineShaderStageCreateInfo> GetStageData() const;
		std::vector<VkPipelineShaderStageCreateInfo> GetStageData(std::vector<KeyValuePair<Shader::Stage, Shader::VkSpecInfo_RAII>> const& specInfo) const;
		//this doesnt need to be explicitly called after construction
		void CreateVkPipeLayout();

		PipeLayout(LogicalDevice& logicalDevice, std::initializer_list<Shader*> shaders);

		//temporarily i need to disable this. i need to figure out how to fix ShaderFactory inclusion dependency
		//PipeLayout(LogicalDevice& logicalDevice, std::initializer_list<std::string_view> shaderFileLocations);

#if PIPELINE_HOT_RELOAD
		void HotReload();
		void DrawImgui();
#endif
#if DEBUG_NAMING
		void SetDebugName(const char* name);
#endif
	};
}