#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/LogicalDevice.h"
#include "EightWinds/Pipeline/Layout.h"

#include "EightWinds/Data/KeyValueContainer.h"

#include <unordered_map>
#include <memory>

namespace EWE {
	struct Pipeline {
		PipeLayout* pipeLayout;
		VkPipeline vkPipe;

		const PipelineID myID;
		PipelineID GetID() const { return myID; };

		Pipeline(PipelineID id, PipeLayout* layout);
		Pipeline(PipelineID id, PipeLayout* layout, std::vector<KeyValuePair<Shader::Stage, std::vector<Shader::SpecializationEntry>>> const& specInfo);

		Pipeline(Pipeline&) = delete;
		Pipeline(Pipeline&&) = delete;
		Pipeline& operator=(Pipeline&) = delete;
		Pipeline& operator=(Pipeline&&) = delete;

#if PIPELINE_HOT_RELOAD
		uint16_t framesSinceSwap = 0;
		VkPipeline stalePipeline = VK_NULL_HANDLE; //going to let it tick for MAX_FRAMES_IN_FLIGHT + 1 then delete it

		virtual void HotReload(bool layoutReload = true) = 0;

		bool enabled = true;
#endif
		std::vector<KeyValuePair<Shader::Stage, std::vector<Shader::SpecializationEntry>>> copySpecInfo;

		void BindDescriptor(uint8_t descSlot, VkDescriptorSet* descSet);
		void BindPipeline();
		void BindPipelineWithVPScissor();

		void Push(void* push, uint8_t pushIndex = 0);
#if DEBUG_NAMING
		void SetDebugName(const char* name);
#endif
	};


}