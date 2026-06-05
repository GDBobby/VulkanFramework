#pragma once
#include "EightWinds/VulkanHeader.h"
#include "EightWinds/LogicalDevice.h"
#include "EightWinds/Shader.h"

#include "EightWinds/Data/KeyValueContainer.h"

#include <initializer_list>
#include <array>
#include <vector>
#include <functional>

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

	PushConstant MergePushRanges(std::span<Shader*> shaders);

	struct PipeLayout {
		LogicalDevice& logicalDevice;
		VkPipelineLayout vkLayout;
		//i suspect theres a mismangement of the Tracker references here
		[[nodiscard]] explicit PipeLayout(LogicalDevice& logicalDevice, std::initializer_list<Shader*> shaders, VkDescriptorSetLayout dsl = VK_NULL_HANDLE) noexcept;

		template<std::size_t... Is>
		static PipeLayout* CreateWithArrayImpl(LogicalDevice& logicalDevice, 
											const std::array<Shader*, ShaderStage::COUNT>& shaders, 
											std::index_sequence<Is...>) {
			return new PipeLayout(logicalDevice, { shaders[Is]... });
		}
		static PipeLayout* DefaultLayoutCreation(LogicalDevice& logicalDevice, std::array<Shader*, ShaderStage::COUNT> shaders){
			return CreateWithArrayImpl(logicalDevice, shaders, std::make_index_sequence<ShaderStage::COUNT>{});
		}

		inline static std::function<PipeLayout*(LogicalDevice& logicalDevice, std::array<Shader*, ShaderStage::COUNT> shaders)> GetLayout = DefaultLayoutCreation;

		//using PipeTraits = PipelineTraits<PipelineType>;
		//i dont like the array much, i might do a KeyValuePair or something
		std::array<Shader*, ShaderStage::COUNT> shaders;

		bool has_bindless_textures = false;
		VkPushConstantRange pushConstantRange{};
		PipelineType pipelineType;
		VkPipelineBindPoint bindPoint; //

		RuntimeArray<VkPipelineShaderStageCreateInfo> GetStageData() const;
		//std::vector<VkPipelineShaderStageCreateInfo> GetStageData(std::vector<KeyValuePair<ShaderStage, VkSpecInfo_RAII>> const& specInfo) const;
		
		//this doesnt need to be explicitly called after construction
		void CreateVkPipeLayout(VkDescriptorSetLayout dsl);

#if PIPELINE_HOT_RELOAD
		void HotReload();
		void DrawImgui();
#endif
		void SetDebugName(std::string_view name);
	};
} //namespace EWE