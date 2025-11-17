#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/LogicalDevice.h"
#include "EightWinds/Pipeline/PipelineBase.h"

namespace EWE {
	struct ComputePipeline : public Pipeline {
		ComputePipeline(LogicalDevice& logicalDevice, PipelineID pipeID, PipeLayout* layout);
		ComputePipeline(LogicalDevice& logicalDevice, PipelineID pipeID, PipeLayout* layout, std::vector<Shader::SpecializationEntry> const& specInfo);

		void CreateVkPipeline();
#if PIPELINE_HOT_RELOAD
		void HotReload(bool layoutReload = true) override final;
#endif
	};
}