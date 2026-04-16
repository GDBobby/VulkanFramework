#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/LogicalDevice.h"
#include "EightWinds/Pipeline/Layout.h"

#include "EightWinds/Data/KeyValueContainer.h"
#include "EightWinds/Command/ParamPacks.h"

#include <memory>

namespace EWE {
	struct Pipeline {
		LogicalDevice& logicalDevice;
		const PipelineID myID;
		PipeLayout* layout; //i dont remember why this is a pointer instead of a reference. i think i was very loosely controlling lifetime?
		VkPipeline vkPipe;

		PipelineID GetID() const { return myID; };

		Pipeline(LogicalDevice& logicalDevice, PipelineID id, PipeLayout* layout);

		//im retiring shader specialization constants for a moment
		//Pipeline(LogicalDevice& logicalDevice, PipelineID id, PipeLayout* layout, KeyValueContainer<ShaderStage, RuntimeArray<Shader::SpecializationEntry>> const& specInfo);

		Pipeline(Pipeline const&) = delete;
		Pipeline(Pipeline&&) = delete;
		Pipeline& operator=(Pipeline const&) = delete;
		Pipeline& operator=(Pipeline&&) = delete;
		
		void WriteToParamPack(ParamPack<Inst::BindPipeline>& paramPack) const;

#if PIPELINE_HOT_RELOAD
		uint16_t framesSinceSwap = 0;
		VkPipeline stalePipeline = VK_NULL_HANDLE; //going to let it tick for MAX_FRAMES_IN_FLIGHT + 1 then delete it

		virtual void HotReload(bool layoutReload = true) = 0;

		bool enabled = true;
#endif
		//im retiring shader specialization constants for at least a moment
		//KeyValueContainer<ShaderStage, RuntimeArray<Shader::SpecializationEntry>> copySpecInfo;
		
#if EWE_DEBUG_NAMING
		void SetDebugName(const char* name);
#endif
	};


}