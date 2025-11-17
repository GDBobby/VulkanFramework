#include "EightWinds/Pipeline/Compute.h"

#if PIPELINE_HOT_RELOAD
#include "EWGraphics/imgui/imgui.h"
#endif

namespace EWE{

	void ComputePipeline::CreateVkPipeline() {
		VkComputePipelineCreateInfo pipelineCreateInfo{};
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.pNext = nullptr;
		pipelineCreateInfo.layout = pipeLayout->vkLayout;
		pipelineCreateInfo.stage = pipeLayout->shaders[Shader::Stage::Compute]->shaderStageCreateInfo;

		Shader::VkSpecInfo_RAII temp{ copySpecInfo[0].value };
		pipelineCreateInfo.stage.pSpecializationInfo = &temp.specInfo;
		EWE_VK(vkCreateComputePipelines, logicalDevice.device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &vkPipe);

	}

	ComputePipeline::ComputePipeline(LogicalDevice& logicalDevice, PipelineID pipeID, PipeLayout* layout) 
		: Pipeline{ logicalDevice, pipeID, layout }
	{
		this->pipeLayout = layout;
		CreateVkPipeline();
	}

	ComputePipeline::ComputePipeline(LogicalDevice& logicalDevice, PipelineID pipeID, PipeLayout* layout, std::vector<Shader::SpecializationEntry> const& specInfo) 
	: 
		Pipeline{ 
			logicalDevice, 
			pipeID, 
			layout, 
			std::vector<KeyValuePair<Shader::Stage, std::vector<Shader::SpecializationEntry>>>{
				KeyValuePair<Shader::Stage, std::vector<Shader::SpecializationEntry>>(Shader::Stage::Compute, specInfo)
			} 
		}
	{
		this->pipeLayout = layout;
		CreateVkPipeline();
	}

#if PIPELINE_HOT_RELOAD
	void ComputePipeline::HotReload(bool layoutReload) {
		//layout should ALWAYS be reloaded
		pipeLayout->HotReload();
		stalePipeline = vkPipe;
		vkPipe = VK_NULL_HANDLE;
		CreateVkPipeline();
	}
#endif
}