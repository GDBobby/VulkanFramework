#include "EightWinds/Pipeline/Graphics.h"

#if PIPELINE_HOT_RELOAD
#include "EWGraphics/Data/magic_enum.hpp"
#include "EWGraphics/imgui/imgui.h"

#include <algorithm>
#endif

#include <cstring> //memcpy

namespace EWE {

#if PIPELINE_HOT_RELOAD
	void imgui_vkbool(std::string_view name, VkBool32& vkBool) {
		bool loe = vkBool;
		ImGui::Checkbox(name.data(), &loe);
		vkBool = loe;
	}

	template<typename T>
	void imgui_enum(std::string_view name, T& val, int min, int max) {

		ImGui::SliderInt(name.data(), reinterpret_cast<int*>(&val), min, max, magic_enum::enum_name(val).data());
	}
	PipelineConfigInfo::PipelineConfigInfo(PipelineConfigInfo const& other) {
		viewportInfo = other.viewportInfo;
		rasterizationInfo = other.rasterizationInfo;
		multisampleInfo = other.multisampleInfo;
		if (other.multisampleInfo.pSampleMask != other.sampleMask) {
			Log::Warning("invalid copy of sampel mask, BE WARNED\n");
		}
		memcpy(sampleMask, other.sampleMask, sizeof(uint32_t) * 2);
		multisampleInfo.pSampleMask = sampleMask;          // Optional

		colorBlendAttachment = other.colorBlendAttachment;
		colorBlendInfo = other.colorBlendInfo;
		colorBlendInfo.pAttachments = &colorBlendAttachment;
		depthStencilInfo = other.depthStencilInfo;

		dynamicStateEnables = other.dynamicStateEnables;
		dynamicStateInfo.sType = other.dynamicStateInfo.sType;
		dynamicStateInfo.pNext = nullptr;
		dynamicStateInfo.pDynamicStates = dynamicStateEnables.data();
		dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
	}
#endif

	void GraphicsPipeline::CreateVkPipeline(
		TaskRasterConfig const& taskConfig,
		ObjectRasterConfig const& objectConfig,
        std::vector<VkDynamicState> const& dynamicState
    ) noexcept {

		//shaders
		/*
		RuntimeArray<KeyValuePair<ShaderStage, Shader::VkSpecInfo_RAII>> temp{copySpecInfo.Size()};
		
		for (std::size_t i = 0; i < copySpecInfo.Size(); i++) {
			temp[i].key = copySpecInfo[i].key;
			temp[i].value = Shader::VkSpecInfo_RAII(copySpecInfo[i].value);
		}
		*/
		std::vector<VkFormat> collected_color_formats{taskConfig.attachment_info.colors.Size()};
		for(std::size_t i = 0; i < collected_color_formats.size(); i++){
			collected_color_formats[i] = taskConfig.attachment_info.colors[i].format;
		}

		VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
			.pNext = nullptr,
			.colorAttachmentCount = static_cast<uint32_t>(collected_color_formats.size()),
			.pColorAttachmentFormats = collected_color_formats.data(),
			.depthAttachmentFormat = taskConfig.attachment_info.using_depth ? taskConfig.attachment_info.depth.format : VK_FORMAT_UNDEFINED,
			.stencilAttachmentFormat = VK_FORMAT_UNDEFINED
		};

		auto shaderStages = layout->GetStageData();//temp);
		VkGraphicsPipelineCreateInfo pipelineCreateInfo{
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.pNext = &pipelineRenderingCreateInfo,
			.stageCount = static_cast<uint32_t>(shaderStages.Size()),
			.pStages = shaderStages.Data(),
			.layout = layout->vkLayout,
			.renderPass = VK_NULL_HANDLE, //DNI
			.subpass = 0, //sub render pass, DNI
			.basePipelineHandle = VK_NULL_HANDLE, //derivates, DNI
			.basePipelineIndex = 0, //derivates, DNI;
		};

        
        //vertex input - im going to enforce programmable vertex pulling (PVP)
        //https://ktstephano.github.io/rendering/opengl/prog_vtx_pulling
        //the way nabla does it (at least in the examples) every vertex is just simplified to vec4s, regardless of real data(each vec3 pos would get a padding of 1 float i guess)
		VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.vertexBindingDescriptionCount = 0,
			.vertexAttributeDescriptionCount = 0
		};
        //the pointers dont matter
        
        //auto& vertShader = layout->shaders[ShaderStage::Vertex];
        //auto vertInputInfo = vertShader.GetVertexInputInfo();
        pipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
		
        //input assembly
		VkPipelineInputAssemblyStateCreateInfo inputAssCreateInfo{
			inputAssCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			inputAssCreateInfo.pNext = nullptr,
			inputAssCreateInfo.flags = 0, //reserved
			inputAssCreateInfo.topology = objectConfig.topology,
			inputAssCreateInfo.primitiveRestartEnable = objectConfig.primitiveRestart ? VK_TRUE : VK_FALSE
		};
		pipelineCreateInfo.pInputAssemblyState = &inputAssCreateInfo;

        pipelineCreateInfo.pTessellationState = nullptr;

		VkPipelineViewportStateCreateInfo viewportStateCreateInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.viewportCount = 1,
			.pViewports = nullptr,
			.scissorCount = 1,
			.pScissors = nullptr,
		};
		pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;

        //const  VkPipelineRasterizationStateCreateInfo *  pRasterizationState;
		VkPipelineRasterizationStateCreateInfo rasterStateCreateInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.depthClampEnable = objectConfig.depthClamp,
			.rasterizerDiscardEnable = objectConfig.rasterizerDiscard,
			.polygonMode = objectConfig.polygonMode,
			.cullMode = objectConfig.cullMode,
			.depthBiasEnable = objectConfig.depthBias.enable,
			.depthBiasConstantFactor = objectConfig.depthBias.constantFactor,
			.depthBiasClamp = objectConfig.depthBias.clamp,
			.depthBiasSlopeFactor = objectConfig.depthBias.slopeFactor,
			.lineWidth = 1.f //this should be a dynamic state if its actually necessary
		};
        pipelineCreateInfo.pRasterizationState = &rasterStateCreateInfo;

        //const  VkPipelineMultisampleStateCreateInfo *  pMultisampleState;
		VkPipelineMultisampleStateCreateInfo msCreateInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			/*
			https://docs.vulkan.org/refpages/latest/refpages/source/VkPipelineMultisampleStateCreateInfo.html
			potential pNext
			*/
			.pNext = nullptr,
			.flags = 0,
			.rasterizationSamples = taskConfig.rastSamples,
			.sampleShadingEnable = taskConfig.enable_sampleShading,
			.minSampleShading = taskConfig.minSampleShading,
			.pSampleMask = nullptr,
			.alphaToCoverageEnable = taskConfig.alphaToCoverageEnable,
			.alphaToOneEnable = taskConfig.alphaToOneEnable
		};
        pipelineCreateInfo.pMultisampleState = &msCreateInfo;

        //const  VkPipelineDepthStencilStateCreateInfo *  pDepthStencilState;
        pipelineCreateInfo.pDepthStencilState = &taskConfig.depthStencilInfo;
        //const  VkPipelineColorBlendStateCreateInfo *  pColorBlendState;
		VkPipelineColorBlendStateCreateInfo blendCreateInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.pNext = nullptr,
			//https://docs.vulkan.org/refpages/latest/refpages/source/VkPipelineColorBlendStateCreateInfo.html - potential flags
			.flags = 0,

			//need to play with these 2, they'll be object based
			.logicOpEnable = false,//maybe, idk
			.logicOp = VK_LOGIC_OP_MAX_ENUM,

			.attachmentCount = static_cast<uint32_t>(taskConfig.attachment_info.colors.Size()),
			.pAttachments = objectConfig.blendAttachments.Data()
		};
		EWE_ASSERT(taskConfig.attachment_info.colors.Size() == objectConfig.blendAttachments.Size());
        memcpy(blendCreateInfo.blendConstants, objectConfig.blendConstants, sizeof(float) * 4);
        pipelineCreateInfo.pColorBlendState = &blendCreateInfo;

        //const  VkPipelineDynamicStateCreateInfo *  pDynamicState;
		VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.dynamicStateCount = static_cast<uint32_t>(taskConfig.dynamicState.size()),
			.pDynamicStates = taskConfig.dynamicState.data()
		};
        pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;

		EWE_VK(vkCreateGraphicsPipelines,
			logicalDevice.device, 
			VK_NULL_HANDLE, 
			1, 
			&pipelineCreateInfo, 
			nullptr, 
			&vkPipe
		);
	}


	GraphicsPipeline::GraphicsPipeline(
        LogicalDevice& _logicalDevice, 
        PipelineID pipeID, 
        PipeLayout* _layout, //the layout SHOULD cover the input assembly
		TaskRasterConfig const& passConfig,
		ObjectRasterConfig const& objectConfig,
        std::vector<VkDynamicState> const& dynamicState//deduced maybe?
    ) noexcept
     : Pipeline{ _logicalDevice, pipeID, _layout }
#if PIPELINE_HOT_RELOAD
		, copyConfigInfo{ configInfo }
#endif
	{
		//read the default spec info
		CreateVkPipeline(passConfig, objectConfig, dynamicState);
	}

	/*
	GraphicsPipeline::GraphicsPipeline(
        LogicalDevice& _logicalDevice, 
        PipelineID pipeID, 
        PipeLayout* _layout, //the layout SHOULD cover the input assembly
		TaskRasterConfig const& passConfig,
        ObjectRasterConfig const& objectConfig,
        std::vector<VkDynamicState> const& dynamicState,//deduced maybe?
    	KeyValueContainer<ShaderStage, RuntimeArray<Shader::SpecializationEntry>> const& specInfo
    ) noexcept
    : Pipeline{ _logicalDevice, pipeID, _layout, specInfo }
#if PIPELINE_HOT_RELOAD
		, copyConfigInfo{ configInfo }
#endif
	{
		CreateVkPipeline(passConfig, objectConfig, dynamicState);
	}
	*/

#if PIPELINE_HOT_RELOAD
	void GraphicsPipeline::HotReload(bool layoutReload) {
		//layout->Reload();
		if (layoutReload) {
			layout->HotReload();
		}
		stalePipeline = vkPipe;
		vkPipe = VK_NULL_HANDLE;
		CreateVkPipeline(copyConfigInfo);
		//std::swap(vkPipe, stalePipeline);
	}
#endif
}