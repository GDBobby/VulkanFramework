#pragma once

//#include "DescriptorHandler.h"
#include "EightWinds/VulkanHeader.h"
#include "EightWinds/LogicalDevice.h"
#include "EightWinds/Backend/Descriptor/SetLayout.h"
#include "EightWinds/Shader.h"
#include "EightWinds/Pipeline/PipelineBase.h"

#include "EightWinds/Pipeline/TaskRasterConfig.h"
#include "EightWinds/ObjectRasterConfig.h"

#include <span>


/*
#define PIPELINE_DERIVATIVES 0 //pipeline derivatives are not currently recommended by hardware vendors
https://developer.nvidia.com/blog/vulkan-dos-donts/ */

//#define DYNAMIC_PIPE_LAYOUT_COUNT 24 //MAX_TEXTURE_COUNT * 4 //defined in descriptorhandler.h

namespace EWE {


	//dropping tess support
	//its not ideal to have less support, BUT
	//i think tess and geo are on the way out.

	//im moving out vertex assembly, attributes, all of it
	//i dont really want pointers in either config. if its unavoidable ill add it
	//but id prefer it to be fully stack

	//this needs a new home
	//std::vector<VkDynamicState> dynamicStateEnables{};
	//VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
	//VkPipelineRenderingCreateInfo pipelineRenderingInfo;


	//each pipeline is going to need four different packages.
	//render info, thats rasterization, blend, samples, etc
	//input info, thats vertex assembly, bindings, attributes
	//dynamic state, dynamic viewport, whatever
    //and VkPipelineRenderingCreateInfo
	struct GraphicsPipeline : public Pipeline {
		[[nodiscard]] GraphicsPipeline(
			LogicalDevice& logicalDevice, 
			PipelineID pipeID, 
			PipeLayout* layout, //the layout SHOULD cover the input assembly
			TaskRasterConfig const& passConfig, ObjectRasterConfig const& objectConfig,
            std::vector<VkDynamicState> const& dynamicState,//deduced maybe?
			std::vector<KeyValuePair<Shader::Stage, std::vector<Shader::SpecializationEntry>>> const& specInfo
		) noexcept;

		[[nodiscard]] GraphicsPipeline(
			LogicalDevice& logicalDevice, 
			PipelineID pipeID, 
            PipeLayout* layout, 
			TaskRasterConfig const& taskConfig, ObjectRasterConfig const& objectConfig,
            std::vector<VkDynamicState> const& dynamicState //deduced maybe?
		) noexcept;

		void CreateVkPipeline(
			TaskRasterConfig const& taskConfig,
			ObjectRasterConfig const& objectConfig,
        	std::vector<VkDynamicState> const& dynamicState
		) noexcept;
		
#if PIPELINE_HOT_RELOAD
		void HotReload(bool layoutReload = true) override final;
		PipelineConfigInfo copyConfigInfo;
#endif
	protected:
		//internal
		//void CreateVkPipeline_SecondStage(PipelineConfigInfo& configInfo, VkGraphicsPipelineCreateInfo& pipelineInfo);
	};
}