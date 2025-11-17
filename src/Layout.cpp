#include "EightWinds/Pipeline/Layout.h"

#include "EightWinds/ShaderFactory.h"

#if PIPELINE_HOT_RELOAD
#include "EWGraphics/imgui/imgui.h"
#endif

#include <algorithm>

namespace EWE {
	Descriptor::LayoutPack MergeDescriptorSets(std::array<Shader*, Shader::Stage::COUNT> const& shaders) {
		assert(shaders.size() > 0);
		Descriptor::LayoutPack ret{};

		uint8_t highestSize = 0;
		for (auto& shader : shaders) {
			if (shader == nullptr) {
				continue;
			}
			highestSize = lab::Max(highestSize, static_cast<uint8_t>(shader->descriptorSets->setLayouts.size()));
		}
		ret.sets.clear();
		ret.sets.reserve(highestSize);

		for (auto& shader : shaders) {
			if (shader == nullptr) {
				continue;
			}
			for (auto& set : shader->descriptorSets.sets) {
				bool hasSetMatch = false;
				for (auto& retSet : ret.sets) {
					if (retSet.index == set.index) {
						hasSetMatch = true;
						auto& retBindings = retSet.bindings;
						auto& shaderBindings = set.bindings;

						for (auto& binding : shaderBindings) {
							bool foundBindingMatch = false;
							for (auto& retBinding : retBindings) {
								if (binding.binding == retBinding.binding) {
									assert(retBinding.descriptorType == binding.descriptorType);
									retBinding.stageFlags |= binding.stageFlags;
									foundBindingMatch = true;
								}
							}
							if (!foundBindingMatch) {
								retBindings.push_back(binding);
							}
						}
						break;
					}
				}
				if (!hasSetMatch) {
					//this needs to be done so it's not moved, and is instead copied
					//i could potentially use std::copy to pass it into the constructor as well
					auto const& constRefBindings = set.value->bindings;

					//ret->setLayouts.push_back(set.key, Construct<Descriptor::SetLayout>(constRefBindings));
                    ret.sets.push_back(set.index, std::copy(constRefBindings));
				}
			}
		}		
		std::sort(ret.sets.begin(), ret.sets.end(),
			[](const Descriptor::Set& a, const Descriptor::Set& b) {
				return a.index < b.index;
			}
		);

		for (auto& dsl : ret.sets) {
			assert(dsl.value->bindings.size() > 0);
            //i need to figure out what to do with this
			dsl.value->BuildVkDSL();
			//std::string debug_name = std::string(fileLocation.data()) + std::string(" - dsl#") + std::to_string(dsl.first);
			//DebugNaming::SetObjectName(dsl.second->vkDSL, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, debug_name.c_str());
		}
		return ret;
	}
	template<typename Container>
	std::vector<VkPushConstantRange> MergePushRanges(Container shaders) {

		std::vector<VkPushConstantRange> ranges{};
		ranges.reserve(shaders.size());
		for (auto& shader : shaders) {
			if (shader == nullptr) {
				continue;
			}
			if (shader->pushRange.size != 0) {
				ranges.push_back(shader->pushRange);
			}
		}
		if (ranges.size() == 0) {
			return {};
		}

		std::sort(ranges.begin(), ranges.end(),
			[](const VkPushConstantRange& a, const VkPushConstantRange& b) {
				if (a.offset != b.offset) return a.offset < b.offset;
				if (a.size != b.size)   return a.size < b.size;
				return a.stageFlags < b.stageFlags;
			}
		);
		std::vector<VkPushConstantRange> merged{};
		for (auto& range : ranges) {
			if (!merged.empty() &&
				merged.back().offset == range.offset &&
				merged.back().size == range.size
			) {
				merged.back().stageFlags |= range.stageFlags;
			}
			else {
				merged.push_back(range);
			}
		}
		return merged;
	}

	PipeLayout::PipeLayout(LogicalDevice& logicalDevice, std::initializer_list<Shader*> shaders)
		: logicalDevice{logicalDevice}
	{
		this->shaders.fill(nullptr);
		for (auto& shader : shaders) {
			this->shaders[Shader::Stage(shader->shaderStageCreateInfo.stage).value] = shader;
		}
		descriptorSets = MergeDescriptorSets(this->shaders);
		pushConstantRanges = MergePushRanges(this->shaders);
		CreateVkPipeLayout();
	}

	PipeLayout::PipeLayout(LogicalDevice& logicalDevice, std::initializer_list<std::string_view> shaderFileLocations)
		: logicalDevice{ logicalDevice }
	{
		this->shaders.fill(nullptr);
		for (auto& fileLocation : shaderFileLocations) {
			Shader* shader = ShaderFactory::GetShader(fileLocation);
			shaders[Shader::Stage(shader->shaderStageCreateInfo.stage).value] = shader;
		}
		descriptorSets = MergeDescriptorSets(this->shaders);
		pushConstantRanges = MergePushRanges(this->shaders);
		CreateVkPipeLayout();
	}

	std::vector<VkPipelineShaderStageCreateInfo> PipeLayout::GetStageData() const {
		std::vector<VkPipelineShaderStageCreateInfo> ret{};
		for (auto& shader : shaders) {
			if (shader != nullptr) {
				ret.push_back(shader->shaderStageCreateInfo);
			}
		}
		return ret;
	}
	std::vector<VkPipelineShaderStageCreateInfo> PipeLayout::GetStageData(std::vector<KeyValuePair<Shader::Stage, Shader::VkSpecInfo_RAII>> const& specInfo) const {
		std::vector<VkPipelineShaderStageCreateInfo> ret{};
		for (auto& shader : shaders) {
			if (shader != nullptr) {
				ret.push_back(shader->shaderStageCreateInfo);
				const Shader::Stage stage(shader->shaderStageCreateInfo.stage);
				for (auto& spec : specInfo) {
					if (spec.key == stage) {
						ret.back().pSpecializationInfo = &spec.value.specInfo;
						break;
					}
				}
			}
		}
		return ret;
	}


	void PipeLayout::CreateVkPipeLayout() {

		if (shaders[Shader::Stage::Vertex] != nullptr) {
			pipelineType = PipelineType::Vertex;
		}
		else if (shaders[Shader::Stage::Mesh] != nullptr) {
			pipelineType = PipelineType::Mesh;
		}
		else if (shaders[Shader::Stage::Compute] != nullptr) {
			pipelineType = PipelineType::Compute;
		}
		else {
			printf("this is going to be raytracing i guess?\n");
			assert(false);
		}


		VkPipelineLayoutCreateInfo plCreateInfo{};
		plCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		plCreateInfo.pNext = nullptr;
		plCreateInfo.flags = 0;
		plCreateInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());
		plCreateInfo.pPushConstantRanges = pushConstantRanges.data();

		if (VK::Object->globalEmptyDSL == VK_NULL_HANDLE) {
			VkDescriptorSetLayoutCreateInfo emptyInfo{};
			emptyInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			emptyInfo.bindingCount = 0;
			emptyInfo.pBindings = nullptr;

			EWE_VK(vkCreateDescriptorSetLayout, logicalDevice.device, &emptyInfo, nullptr, &VK::Object->globalEmptyDSL);
		}
		uint8_t highestCount = 0;
		for (auto& dsl : descriptorSets.sets) {
			highestCount = lab::Max(highestCount, dsl.index); //theyre sorted, i need to add back() to KVContainer
		}
		std::vector<VkDescriptorSetLayout> layouts(highestCount + 1, VK::Object->globalEmptyDSL);
		for (uint8_t i = 0; i < descriptorSets.sets.size(); i++) {
            //this needs to be dealt with
			layouts[descriptorSets.sets[i].index] = descriptorSets->sets[i].bindings->vkDSL;
		}

		plCreateInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
		plCreateInfo.pSetLayouts = layouts.data();

		EWE_VK(vkCreatePipelineLayout, logicalDevice.device, &plCreateInfo, nullptr, &vkLayout);
	}

#if PIPELINE_HOT_RELOAD
	void PipeLayout::HotReload() {
		for (auto& shader : shaders) {
			if (shader != nullptr) {
				shader->HotReload();
			}
		}
		printf("memory leak here - descriptor sets not freed before recreated\n");
		//Deconstruct(descriptorSets); //need this to be stored then deleted stale
		pushConstantRanges.clear();

		descriptorSets = MergeDescriptorSets(this->shaders);
		pushConstantRanges = MergePushRanges(this->shaders);
		CreateVkPipeLayout();
	}

	void PipeLayout::DrawImgui() {
		for (uint8_t i = 0; i < shaders.size(); i++) {
			//ImGui::InputText(magic_enum::enum_name(static_cast<Shader::Stage::Bits>(i)).data(), shaders[i]->filepath.c_str());
		}
	}
#endif

#if DEBUG_NAMING
	void PipeLayout::SetDebugName(const char* name) {
		DebugNaming::SetObjectName(vkLayout, VK_OBJECT_TYPE_PIPELINE_LAYOUT, name);
	}
#endif
}