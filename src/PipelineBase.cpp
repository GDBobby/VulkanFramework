#include "EightWinds/Pipeline/PipelineBase.h"


//#include "EWGraphics/Texture/Image_Manager.h"

#if PIPELINE_HOT_RELOAD
#include "EWGraphics/imgui/imgui.h"
#include "EWGraphics/Vulkan/GraphicsPipeline.h"
#include "EWGraphics/Vulkan/ComputePipeline.h"
#endif

#include <unordered_map>

namespace EWE {


//#if PIPELINE_HOT_RELOAD
	std::vector<KeyValuePair<Shader::Stage, std::vector<Shader::SpecializationEntry>>> SpecInitializer(PipeLayout* layout) {
		uint8_t shaderCount = 0;
		std::vector<KeyValuePair<Shader::Stage, std::vector<Shader::SpecializationEntry>>> ret{};
		for (auto& shader : layout->shaders) {
			if (shader != nullptr) {
				ret.push_back(KeyValuePair<Shader::Stage, std::vector<Shader::SpecializationEntry>>(Shader::Stage(shader->shaderStageCreateInfo.stage), shader->defaultSpecConstants));
			}
		}
		return ret;
	}
//#endif

	Pipeline::Pipeline(LogicalDevice& logicalDevice, PipelineID id, PipeLayout* layout) 
	: 
		logicalDevice{logicalDevice},
		myID{ id },
		pipeLayout{ layout }
//#if PIPELINE_HOT_RELOAD
		, copySpecInfo{ SpecInitializer(layout) }
//#endif
	{}

	Pipeline::Pipeline(LogicalDevice& logicalDevice, PipelineID pipeID, PipeLayout* layout, std::vector<KeyValuePair<Shader::Stage, std::vector<Shader::SpecializationEntry>>> const& specInfo)
	: 
		logicalDevice{logicalDevice},
		myID{ pipeID }, 
		pipeLayout{ layout }, 
		copySpecInfo{ specInfo }
	{}


	void Pipeline::BindDescriptor(VkCommandBuffer cmdBuf, uint8_t descSlot, VkDescriptorSet* descSet) {
		vkCmdBindDescriptorSets(cmdBuf,
			pipeLayout->bindPoint,
			pipeLayout->vkLayout,
			descSlot, 1,
			descSet,
			0, nullptr
		);
	}

	void Pipeline::BindPipeline(VkCommandBuffer cmdBuf) {
		vkCmdBindPipeline(cmdBuf, pipeLayout->bindPoint, vkPipe);
	}
	//void Pipeline::BindPipelineWithVPScissor() {
	//	BindPipeline();
	//	EWE_VK(vkCmdSetViewport, VK::Object->GetFrameBuffer(), 0, 1, &VK::Object->viewport);
	//	EWE_VK(vkCmdSetScissor, VK::Object->GetFrameBuffer(), 0, 1, &VK::Object->scissor);
	//}
	void Pipeline::Push(VkCommandBuffer cmdBuf, void* push, uint8_t pushIndex) {
		auto& range = pipeLayout->pushConstantRanges[pushIndex];
		vkCmdPushConstants(cmdBuf, pipeLayout->vkLayout, range.stageFlags, range.offset, range.size, push);
	}

#if EWE_DEBUG_NAMING
	void Pipeline::SetDebugName(const char* name) {
		logicalDevice.SetObjectName(vkPipe, VK_OBJECT_TYPE_PIPELINE, name);
		pipeLayout->SetDebugName(name);
	}
#endif
}