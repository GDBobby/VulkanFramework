#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/Pipeline/PipelineBase.h"

namespace EWE{
    //need to make this an object instead of a namespace
	//same problem as factory, where the best place to put it would be logicaldevice
	//but logicaldevice is included by what this includes
	struct PipelineSystem {
		LogicalDevice& logicalDevice;

		Pipeline* At(PipelineID pipeID);

#if DEBUG_NAMING
		void Emplace(std::string const& pipeName, PipelineID pipeID, Pipeline* pipe);

		template<typename T>
		void Emplace(T pipeID, Pipeline* pipeSys) {
			const std::string pipeName = std::string(magic_enum::enum_name(pipeID));
			Emplace(pipeName, pipeID, pipeSys);
		}
#else
		void Emplace(PipelineID pipeID, Pipeline* pipeSys);
#endif
		void DestructAll();

		bool OptionalDestructAt(PipelineID pipeID);
		void DestructAt(PipelineID pipeID);
#if PIPELINE_HOT_RELOAD
		void DrawImgui();
#endif

        private:
        
		std::unordered_map<PipelineID, ::EWE::Pipeline*> pipelineMap{};
		std::unordered_map<PipelineID, std::string> pipelineNames{};
	};
}