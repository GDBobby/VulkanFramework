#pragma once
#include "EightWinds/VulkanHeader.h"
#include "EightWinds/LogicalDevice.h"
#include "EightWinds/Shader.h"
#include "EightWinds/DescriptorSetLayout.h"

#include "EightWinds/Data/KeyValueContainer.h"

#include <initializer_list>
#include <array>
#include <vector>

namespace EWE {
	enum class PipelineType {
		Vertex, //combination of vertex, fragment - optional geom/tess
		Compute,
		Mesh, //task mesh
		//MeshWithMeshDisabled, //theoretical at the moment
		Raytracing, //any RT combo
		Scheduler, //graph/scheduling - distinct? idk

		COUNT
	};

	constexpr VkPipelineBindPoint BindPointFromType(PipelineType pt) {
		switch (pt) {
		case PipelineType::Vertex: return VK_PIPELINE_BIND_POINT_GRAPHICS;
		case PipelineType::Compute: return VK_PIPELINE_BIND_POINT_COMPUTE;
		case PipelineType::Mesh: return VK_PIPELINE_BIND_POINT_GRAPHICS;
		//case PipelineType::MeshWithMeshDisabled: return VK_PIPELINE_BIND_POINT_GRAPHICS; //this is a bit more ambiguous
		case PipelineType::Raytracing: return VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
			//Scheduler, //graph/scheduling - distinct? idk
		}
		EWE_UNREACHABLE;
	}

	struct PipeLayout {
		LogicalDevice& logicalDevice;
		VkPipelineLayout vkLayout;
		//i suspect theres a mismangement of the Tracker references here
		[[nodiscard]] explicit PipeLayout(LogicalDevice& logicalDevice, std::initializer_list<::EWE::Shader*> shaders) noexcept;
		//temporarily i need to disable this. i need to figure out how to fix ShaderFactory inclusion dependency
		//PipeLayout(LogicalDevice& logicalDevice, std::initializer_list<std::string_view> shaderFileLocations);

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



#if PIPELINE_HOT_RELOAD
		void HotReload();
		void DrawImgui();
#endif
#if DEBUG_NAMING
		void SetDebugName(const char* name);
#endif
	};
} //namespace EWE