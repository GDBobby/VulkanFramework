#include "EightWinds/Pipeline/Graphics.h"

#if PIPELINE_HOT_RELOAD
#include "EWGraphics/Data/magic_enum.hpp"
#include "EWGraphics/imgui/imgui.h"

#include <algorithm>
#endif

#include <cassert>
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
			printf("invalid copy of sampel mask, BE WARNED\n");
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
	/*
	void PipelineConfigInfo::RenderIMGUI() {
		static int currentImguiIndex = 0;
		if (imguiIndex < 0) {
			imguiIndex = currentImguiIndex;
			currentImguiIndex++;
		}


		std::string extension = "##p";
		extension += std::to_string(imguiIndex);

		std::string treeName = "pipeline config info";
		treeName += extension;

		std::string optionStr;
		if (ImGui::TreeNode(treeName.c_str())) {

			optionStr = "viewport info";
			optionStr += extension;
			if (ImGui::TreeNode(optionStr.c_str())) {

				optionStr = "topology";
				optionStr += extension;
				imgui_enum(optionStr, inputAssemblyInfo.topology, 0, 10);

				optionStr = "primitive restart enable";
				optionStr += extension;
				imgui_vkbool(optionStr, inputAssemblyInfo.primitiveRestartEnable);

				ImGui::TreePop();
			}
			//input assembly
			optionStr = "rasterzation info";
			optionStr += extension;
			if (ImGui::TreeNode(optionStr.c_str())) {

				optionStr = "depth clamp enable";
				optionStr += extension;
				imgui_vkbool(optionStr, rasterizationInfo.depthClampEnable);

				optionStr = "rasterizer discard enable";
				optionStr += extension;
				imgui_vkbool(optionStr, rasterizationInfo.rasterizerDiscardEnable);

				optionStr = "polygonMode";
				optionStr += extension;
				imgui_enum(optionStr, rasterizationInfo.polygonMode, 0, 2);

				optionStr = "cullMode";
				optionStr += extension;
				ImGui::DragInt(optionStr.c_str(), reinterpret_cast<int*>(&rasterizationInfo.cullMode), 1, 0, 100);
				VkCullModeFlagBits copyCull = static_cast<VkCullModeFlagBits>(rasterizationInfo.cullMode);
				imgui_enum(optionStr.c_str(), copyCull, 0, 3);
				rasterizationInfo.cullMode = copyCull;

				optionStr = "frontFace";
				optionStr += extension;
				imgui_enum(optionStr, rasterizationInfo.frontFace, 0, 1);

				optionStr = "depthBiasEnable";
				optionStr += extension;
				imgui_vkbool(optionStr, rasterizationInfo.depthBiasEnable);

				optionStr = "depth bias constant factor";
				optionStr += extension;
				ImGui::DragFloat(optionStr.c_str(), &rasterizationInfo.depthBiasConstantFactor, 0.1f, 0.f, 100.f);

				optionStr = "depth bias clamp";
				optionStr += extension;
				ImGui::DragFloat(optionStr.c_str(), &rasterizationInfo.depthBiasClamp, 0.1f, 0.f, 100.f);

				optionStr = "depth bias slope factor";
				optionStr += extension;
				ImGui::DragFloat(optionStr.c_str(), &rasterizationInfo.depthBiasSlopeFactor, 0.1f, 0.f, 100.f);

				optionStr = "line width";
				optionStr += extension;
				ImGui::DragFloat(optionStr.c_str(), &rasterizationInfo.lineWidth, 0.1f, 0.f, 100.f);


				ImGui::TreePop();
			}

			optionStr = "color blend";
			optionStr += extension;
			if (ImGui::TreeNode(optionStr.c_str())) {

				optionStr = "blend constants";
				optionStr += extension;
				ImGui::DragFloat4(optionStr.c_str(), colorBlendInfo.blendConstants, 0.01f, 0.f, 1.f);

				optionStr = "logic op";
				optionStr += extension;
				//ImGui::SliderInt(optionStr.c_str(), reinterpret_cast<int*>(&colorBlendInfo.logicOp), 0, 15, magic_enum::enum_name(colorBlendInfo.logicOp).data());
				imgui_enum(optionStr, colorBlendInfo.logicOp, 0, 15);

				optionStr = "logic op enable";
				optionStr += extension;
				imgui_vkbool(optionStr, colorBlendInfo.logicOpEnable);
				ImGui::TreePop();
			}
			optionStr = "dynamic states";
			optionStr += extension;
			if (ImGui::TreeNode(optionStr.c_str())) {
				for (auto const& dynState : dynamicStateEnables) {
					ImGui::Text("%s - %d", magic_enum::enum_name(dynState).data(), dynState);
				}
				ImGui::TreePop();
			}

			//if(ImGui::TreeNode(optionStr.c_str(), ))

			ImGui::TreePop();
		}


	}
    */

	void GraphicsPipeline::CreateVkPipeline(
		TaskRasterConfig const& taskConfig,
		ObjectRasterConfig const& objectConfig,
        std::vector<VkDynamicState> const& dynamicState
    ) noexcept {

		//shaders
		std::vector<KeyValuePair<Shader::Stage, Shader::VkSpecInfo_RAII>> temp{};
		for (auto& stage : copySpecInfo) {
			temp.push_back(KeyValuePair<Shader::Stage, Shader::VkSpecInfo_RAII>(stage.key, Shader::VkSpecInfo_RAII(stage.value)));
		}
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages = pipeLayout->GetStageData(temp);
		VkGraphicsPipelineCreateInfo pipelineCreateInfo{
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.pNext = &taskConfig.pipelineRenderingCreateInfo,
			.stageCount = static_cast<uint32_t>(shaderStages.size()),
			.pStages = shaderStages.data(),
			.layout = pipeLayout->vkLayout,
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
        
        //auto& vertShader = pipeLayout->shaders[Shader::Stage::Vertex];
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

			.attachmentCount = static_cast<uint32_t>(taskConfig.attachment_set_info.colors.size()),
			.pAttachments = &objectConfig.blendAttachment
		};
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
        LogicalDevice& logicalDevice, 
        PipelineID pipeID, 
        PipeLayout* layout, //the layout SHOULD cover the input assembly
		TaskRasterConfig const& passConfig,
		ObjectRasterConfig const& objectConfig,
        std::vector<VkDynamicState> const& dynamicState//deduced maybe?
    ) noexcept
     : Pipeline{ logicalDevice, pipeID, layout }
#if PIPELINE_HOT_RELOAD
		, copyConfigInfo{ configInfo }
#endif
	{
		//read the default spec info
		CreateVkPipeline(passConfig, objectConfig, dynamicState);
	}
	GraphicsPipeline::GraphicsPipeline(
        LogicalDevice& logicalDevice, 
        PipelineID pipeID, 
        PipeLayout* layout, //the layout SHOULD cover the input assembly
		TaskRasterConfig const& passConfig,
        ObjectRasterConfig const& objectConfig,
        std::vector<VkDynamicState> const& dynamicState,//deduced maybe?
        std::vector<KeyValuePair<Shader::Stage, std::vector<Shader::SpecializationEntry>>> const& specInfo
    ) noexcept
    : Pipeline{ logicalDevice, pipeID, layout, specInfo }
#if PIPELINE_HOT_RELOAD
		, copyConfigInfo{ configInfo }
#endif
	{
		CreateVkPipeline(passConfig, objectConfig, dynamicState);
	}

#if PIPELINE_HOT_RELOAD
	void GraphicsPipeline::HotReload(bool layoutReload) {
		//layout->Reload();
		if (layoutReload) {
			pipeLayout->HotReload();
		}
		stalePipeline = vkPipe;
		vkPipe = VK_NULL_HANDLE;
		CreateVkPipeline(copyConfigInfo);
		//std::swap(vkPipe, stalePipeline);
	}
#endif
}