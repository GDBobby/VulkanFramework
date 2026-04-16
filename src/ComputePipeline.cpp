#include "EightWinds/Pipeline/Compute.h"

#if PIPELINE_HOT_RELOAD
#include "EWGraphics/imgui/imgui.h"
#endif

namespace EWE{

	void ComputePipeline::CreateVkPipeline() {
		VkComputePipelineCreateInfo pipelineCreateInfo{
			.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
			.pNext = nullptr,
			.stage = layout->shaders[ShaderStage::Compute]->shaderStageCreateInfo,
			.layout = layout->vkLayout
		};
		/*
		if(copySpecInfo.size() > 0){
			Shader::VkSpecInfo_RAII temp{ copySpecInfo[0].value };
			pipelineCreateInfo.stage.pSpecializationInfo = &temp.specInfo;
		}
		*/
		EWE_VK(vkCreateComputePipelines, logicalDevice.device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &vkPipe);

	}

	ComputePipeline::ComputePipeline(LogicalDevice& _logicalDevice, PipelineID pipeID, PipeLayout* _layout) 
		: Pipeline{ _logicalDevice, pipeID, _layout }
	{
		CreateVkPipeline();
	}

	/*
	ComputePipeline::ComputePipeline(LogicalDevice& _logicalDevice, PipelineID pipeID, PipeLayout* _layout, std::vector<Shader::SpecializationEntry> const& specInfo) 
	: 
		Pipeline{ 
			_logicalDevice, 
			pipeID, 
			_layout
			//, 
			//KeyValueContainer<ShaderStage, RuntimeArray<Shader::SpecializationEntry>>{} 
		}
	{

		//copySpecInfo[0].key = ShaderStage::Compute;
		//copySpecInfo[0].value = specInfo;
		layout = _layout;
		CreateVkPipeline();
	}
	*/

#if PIPELINE_HOT_RELOAD
	void ComputePipeline::HotReload(bool layoutReload) {
		//layout should ALWAYS be reloaded
		layout->HotReload();
		stalePipeline = vkPipe;
		vkPipe = VK_NULL_HANDLE;
		CreateVkPipeline();
	}
#endif
}