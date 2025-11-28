#pragma once

//#include "DescriptorHandler.h"
#include "EightWinds/VulkanHeader.h"
#include "EightWinds/LogicalDevice.h"
#include "EightWinds/Backend/Descriptor/SetLayout.h"
#include "EightWinds/Shader.h"
#include "EightWinds/Pipeline/PipelineBase.h"


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

	//i can split the full VkPipelineCreateInfo into per pass, and per object
	//the idea is i can pass in an object with  
	
	//per pass is going to cover per viewport and all that as well
	//how each pass is configured
	struct PipelinePassConfig{
		uint32_t viewportCount = 1;//not going to be changed in 99.9% of games
		uint32_t scissorCount = 1; //not going to be changed in 99.9% of games
		VkSampleCountFlagBits rastSamples;
		bool enable_sampleShading;
			//depends on ^
			//{
			float minSampleShading;
			//}
		bool alphaToCoverageEnable;
		bool alphaToOneEnable;
		std::vector<VkDynamicState> dynamicState{};
		
		//VkPipelineViewportStateCreateInfo viewportInfo{}; //condensed to just vp/scissor count
		
		//this can be reduced to MSAA, pNext and flags are useless (nvidia has some extensions)
		//VkPipelineMultisampleStateCreateInfo multisampleInfo{}; 
		
		//im not going to allow specific sample selection. it might be useful but idk what for. seems like minSampleShading is the same thing. idk

		//depth controlled exclusively by the pass?
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
		VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo{};
		//^these are going to be defined externally, in the rendergraph i think.
		//the user wont put their hands on this
		//im going to leave it as is for the moment, it can probably be simplified

		void SetDefaults() noexcept;

	};

	struct DepthBias {
		bool  enable;
		float constantFactor;
		float clamp;
		float slopeFactor;
	};
	//how each object is configured
	struct PipelineObjectConfig{
		bool depthClamp;
		bool rasterizerDiscard;
		VkPolygonMode polygonMode;
		VkCullModeFlags cullMode;
		VkFrontFace frontFace;
		DepthBias depthBias;
		VkPrimitiveTopology topology;
		bool primitiveRestart;

		//i think im only going to allow 1 blend attachment max
		//if its disabled, blendAttachment.blendEnabled, it wont be added to blendStateCreateInfo
		//i think this is per object but im not sure
		VkPipelineColorBlendAttachmentState blendAttachment;

		//i need to mess with this
		float blendConstants[4];

		void SetDefaults() noexcept;

		//VkLogicOp blendLogicOp; //need to play iwth this
	};

	/*
	struct PipelineConfigInfo {
		PipelineConfigInfo() = default;
		void RenderIMGUI();
		int16_t imguiIndex = -1;
#if PIPELINE_HOT_RELOAD
		PipelineConfigInfo(PipelineConfigInfo const&);
#else
		PipelineConfigInfo(const PipelineConfigInfo&) = delete;
#endif
		PipelineConfigInfo& operator=(PipelineConfigInfo const&) = delete;
		PipelineConfigInfo(PipelineConfigInfo&&) = delete;
		PipelineConfigInfo& operator=(PipelineConfigInfo&&) = delete;

		void EnableAlphaBlending();
		void Enable2DConfig();
		//static PipelineConfigInfo DefaultPipelineConfigInfo(); //requires the move operator? something RVO
		void SetToDefaults();

		//i can compact a lot of this information
		VkPipelineViewportStateCreateInfo viewportInfo{};
		//^i think viewport info can go away
		VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
		VkPipelineMultisampleStateCreateInfo multisampleInfo{};
		uint32_t sampleMask[2];
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};

		void* pNext;
	};
	*/


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
			PipelinePassConfig const& passConfig, 
			PipelineObjectConfig const& objectConfig,
            std::vector<VkDynamicState> const& dynamicState,//deduced maybe?
			std::vector<KeyValuePair<Shader::Stage, std::vector<Shader::SpecializationEntry>>> const& specInfo
		) noexcept;

		[[nodiscard]] GraphicsPipeline(
			LogicalDevice& logicalDevice, 
			PipelineID pipeID, 
            PipeLayout* layout, 
			PipelinePassConfig const& passConfig, 
			PipelineObjectConfig const& objectConfig,
            std::vector<VkDynamicState> const& dynamicState //deduced maybe?
		) noexcept;

		void CreateVkPipeline(
			PipelinePassConfig const& passConfig, 
        	PipelineObjectConfig const& objectConfig,
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