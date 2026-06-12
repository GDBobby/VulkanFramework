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
		depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

		//need to play with all of this
		depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
		depthStencilInfo.stencilTestEnable = VK_FALSE;
		depthStencilInfo.front = {};  // Optional
		depthStencilInfo.back = {};   // Optional
		depthStencilInfo.minDepthBounds = 0.0f;  // Optional
		depthStencilInfo.maxDepthBounds = 1.0f;  // Optional

		attachment_info.relative_size = true;
		attachment_info.width = 1.f;
		attachment_info.height = 1.f;
		attachment_info.renderingFlags = 0;
		attachment_info.colors.ClearAndResize(1);
		attachment_info.colors[0].format = VK_FORMAT_R8G8B8A8_UNORM;
		attachment_info.colors[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment_info.colors[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment_info.colors[0].clearValue.color = {0.f, 0.f, 0.f, 0.f};

		attachment_info.using_depth = true;
		attachment_info.depth.format = VK_FORMAT_D16_UNORM;
		attachment_info.depth.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		attachment_info.depth.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment_info.depth.clearValue.color = {0.f, 0.f, 0.f, 0.f};
	}


	TaskRasterConfig TaskRasterConfig::GetDefault() {
		TaskRasterConfig ret{};
		ret.SetDefaults();
		return ret;
	}
}