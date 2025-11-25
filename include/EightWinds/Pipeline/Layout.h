#pragma once
#include "EightWinds/VulkanHeader.h"
#include "EightWinds/LogicalDevice.h"
#include "EightWinds/Shader.h"
#include "EightWinds/Backend/Descriptors/SetLayout.h"
#include "EightWinds/Framework.h"

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


	struct PipeLayout {
		Framework& framework;
		VkPipelineLayout vkLayout;
		//i suspect theres a mismangement of the Tracker references here
		[[nodiscard]] explicit PipeLayout(Framework& framework, std::initializer_list<::EWE::Shader*> shaders) noexcept;
		//temporarily i need to disable this. i need to figure out how to fix ShaderFactory inclusion dependency
		[[nodiscard]] explicit PipeLayout(Framework& framework, std::initializer_list<std::string_view> shaderFileLocations);

		//using PipeTraits = PipelineTraits<PipelineType>;
		//i dont like the array much, i might do a KeyValuePair or something
		std::array<Shader*, Shader::Stage::COUNT> shaders;

		Descriptor::LayoutPack descriptorSets;
		std::vector<VkPushConstantRange> pushConstantRanges{};
		PipelineType pipelineType;
		VkPipelineBindPoint bindPoint; //

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