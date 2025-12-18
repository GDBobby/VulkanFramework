#pragma once
#include "EightWinds/VulkanHeader.h"
#include "EightWinds/LogicalDevice.h"
#include "EightWinds/Shader.h"
#include "EightWinds/Backend/Descriptor/SetLayout.h"

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
		LogicalDevice& logicalDevice;
		VkPipelineLayout vkLayout;
		//i suspect theres a mismangement of the Tracker references here
		[[nodiscard]] explicit PipeLayout(LogicalDevice& logicalDevice, std::initializer_list<::EWE::Shader*> shaders, VkDescriptorSetLayout dsl = VK_NULL_HANDLE) noexcept;

		//using PipeTraits = PipelineTraits<PipelineType>;
		//i dont like the array much, i might do a KeyValuePair or something
		std::array<Shader*, Shader::Stage::COUNT> shaders;

		Backend::Descriptor::LayoutPack descriptorSets;
		std::vector<VkPushConstantRange> pushConstantRanges{};
		PipelineType pipelineType;
		VkPipelineBindPoint bindPoint; //

		std::vector<VkPipelineShaderStageCreateInfo> GetStageData() const;
		std::vector<VkPipelineShaderStageCreateInfo> GetStageData(std::vector<KeyValuePair<Shader::Stage, Shader::VkSpecInfo_RAII>> const& specInfo) const;
		//this doesnt need to be explicitly called after construction
		void CreateVkPipeLayout(VkDescriptorSetLayout dsl);



#if PIPELINE_HOT_RELOAD
		void HotReload();
		void DrawImgui();
#endif
#if EWE_DEBUG_NAMING
		void SetDebugName(const char* name);
#endif
	};
} //namespace EWE