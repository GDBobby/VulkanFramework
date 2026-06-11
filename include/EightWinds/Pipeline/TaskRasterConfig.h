#pragma once
#include "EightWinds/VulkanHeader.h"
#include "EightWinds/Backend/RenderInfo.h"

namespace EWE{

	//i can split the full VkPipelineCreateInfo into per pass, and per object
	//the idea is i can pass in an object with  
	
	//per pass is going to cover per viewport and all that as well
	//how each pass is configured
	struct TaskRasterConfig{
		//viewport/scissor not going to be changed in 99.9% of games
		uint32_t viewportCount = 1;
		uint32_t scissorCount = 1;

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
		AttachmentSetInfo attachment_info;
		//^these are going to be defined externally, in the rendergraph i think.
		//the user wont put their hands on this
		//im going to leave it as is for the moment, it can probably be simplified

		void SetDefaults() noexcept;


		static void WriteToFile(TaskRasterConfig const& config, std::ofstream& outFile);
		static void ReadFromFile(TaskRasterConfig& config, std::ifstream& inFile);
	};

}