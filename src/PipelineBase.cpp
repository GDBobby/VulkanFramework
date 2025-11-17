#include "EightWinds/PipelineBase.h"


//#include "EWGraphics/Texture/Image_Manager.h"

#if PIPELINE_HOT_RELOAD
#include "EWGraphics/imgui/imgui.h"
#include "EWGraphics/Vulkan/GraphicsPipeline.h"
#include "EWGraphics/Vulkan/ComputePipeline.h"
#endif

#include <unordered_map>

namespace EWE {


#if PIPELINE_HOT_RELOAD
	std::vector<KeyValuePair<ShaderStage, std::vector<Shader::SpecializationEntry>>> SpecInitializer(PipeLayout* layout) {
		uint8_t shaderCount = 0;
		std::vector<KeyValuePair<ShaderStage, std::vector<Shader::SpecializationEntry>>> ret{};
		for (auto& shader : layout->shaders) {
			if (shader != nullptr) {
				ret.push_back(KeyValuePair<ShaderStage, std::vector<Shader::SpecializationEntry>>(ShaderStage(shader->shaderStageCreateInfo.stage), shader->defaultSpecConstants));
			}
		}
		return ret;
	}
#endif

	Pipeline::Pipeline(PipelineID id, PipeLayout* layout) : myID{ id },
		pipeLayout{ layout }, 
		copySpecInfo{ SpecInitializer(layout) }
	{}

	Pipeline::Pipeline(PipelineID pipeID, PipeLayout* layout, std::vector<KeyValuePair<Shader::Stage, std::vector<Shader::SpecializationEntry>>> const& specInfo)
		: myID{ pipeID }, 
		pipeLayout{ layout }, 
		copySpecInfo{ specInfo }
	{}


	void Pipeline::BindDescriptor(uint8_t descSlot, VkDescriptorSet* descSet) {
		EWE_VK(vkCmdBindDescriptorSets, VK::Object->GetFrameBuffer(),
			BindPointFromType(pipeLayout->pipelineType),
			pipeLayout->vkLayout,
			descSlot, 1,
			descSet,
			0, nullptr
		);
	}

	void Pipeline::BindPipeline() {
		EWE_VK(vkCmdBindPipeline, VK::Object->GetFrameBuffer(), BindPointFromType(pipeLayout->pipelineType), vkPipe);
	}
	void Pipeline::BindPipelineWithVPScissor() {
		BindPipeline();
		EWE_VK(vkCmdSetViewport, VK::Object->GetFrameBuffer(), 0, 1, &VK::Object->viewport);
		EWE_VK(vkCmdSetScissor, VK::Object->GetFrameBuffer(), 0, 1, &VK::Object->scissor);
	}
	void Pipeline::Push(void* push, uint8_t pushIndex) {
		auto& range = pipeLayout->pushConstantRanges[pushIndex];
		EWE_VK(vkCmdPushConstants, VK::Object->GetFrameBuffer(), pipeLayout->vkLayout, range.stageFlags, range.offset, range.size, push);
	}

#if DEBUG_NAMING
	void Pipeline::SetDebugName(const char* name) {
		DebugNaming::SetObjectName(vkPipe, VK_OBJECT_TYPE_PIPELINE, name);
		pipeLayout->SetDebugName(name);
	}
#endif
}