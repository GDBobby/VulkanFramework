#include "EightWinds/Pipeline/TaskRasterConfig.h"

namespace EWE{
	
	void TaskRasterConfig::SetDefaults() noexcept {
		viewportCount = 1;
		scissorCount = 1;
		rastSamples = VK_SAMPLE_COUNT_1_BIT;
		enable_sampleShading = false;
		minSampleShading = 1.f;
		alphaToCoverageEnable = VK_FALSE;
		alphaToOneEnable = VK_FALSE;

		dynamicState = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

		depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilInfo.pNext = nullptr;
		depthStencilInfo.flags = 0;
		depthStencilInfo.depthTestEnable = VK_TRUE;
		depthStencilInfo.depthWriteEnable = VK_TRUE;
		depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;

		//need to play with all of this
		depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
		depthStencilInfo.stencilTestEnable = VK_FALSE;
		depthStencilInfo.front = {};  // Optional
		depthStencilInfo.back = {};   // Optional
		depthStencilInfo.minDepthBounds = 0.0f;  // Optional
		depthStencilInfo.maxDepthBounds = 1.0f;  // Optional

		//no valid defaults, this needs real data
		pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		pipelineRenderingCreateInfo.pNext = nullptr;
		attachment_set_info.colors.clear();
		attachment_set_info.colors.push_back(
			AttachmentInfo{ 
				.format = VK_FORMAT_R8G8B8A8_UNORM, 
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.clearValue = {0.f, 0.f, 0.f, 0.f}
				 
			}
		);
		pipelineRenderingCreateInfo.colorAttachmentCount = static_cast<uint32_t>(attachment_set_info.colors.size());
		pipelineRenderingCreateInfo.depthAttachmentFormat = attachment_set_info.depth.format;
		pipelineRenderingCreateInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
	}

}