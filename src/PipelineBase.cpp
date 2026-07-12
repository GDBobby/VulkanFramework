#include "EightWinds/Pipeline/PipelineBase.h"


//#include "EWGraphics/Texture/Image_Manager.h"

#if PIPELINE_HOT_RELOAD
#include "EWGraphics/imgui/imgui.h"
#include "EWGraphics/Vulkan/GraphicsPipeline.h"
#include "EWGraphics/Vulkan/ComputePipeline.h"
#endif

namespace EWE {
	
	
	void Pipeline::WriteToParamPack(ParamPack<Inst::BindPipeline>& paramPack) const{
		paramPack.pipe = pipe;
		paramPack.layout = layout->layout;
		paramPack.bindPoint = layout->bindPoint;
	}

/*
	KeyValueContainer<ShaderStage, RuntimeArray<Shader::SpecializationEntry>> SpecInitializer(PipeLayout* layout) {
		
		uint8_t shader_count = 0;
		for(auto& shader : shaders){
			shader_count += shader != nullptr;
		}

		RuntimeArray<KeyValuePair<ShaderStage, RuntimeArray<Shader::SpecializationEntry>>> ret{shader_count};
		uint8_t current_shader_length = 0;
		for (auto& shader : layout->shaders) {
			if (shader != nullptr) {
				ret[current_shader_length].key = ShaderStage(shader->shaderStageCreateInfo.stage);
				ret[current_shader_length].value = shader->defaultSpecConstants;
			}
		}
		return ret;
	}
*/

	Pipeline::Pipeline(LogicalDevice& _logicalDevice, PipeLayout* _layout) 
	: 
		logicalDevice{_logicalDevice},
		layout{ _layout }
		//, copySpecInfo{ SpecInitializer(layout) }
	{}

	/*
	Pipeline::Pipeline(LogicalDevice& _logicalDevice, PipelineID pipeID, PipeLayout* _layout, KeyValueContainer<ShaderStage, RuntimeArray<Shader::SpecializationEntry>> const& specInfo)
	: logicalDevice{_logicalDevice},
		myID{ pipeID }, 
		layout{ _layout },
		copySpecInfo{ specInfo }
	{}
	*/

	void Pipeline::SetDebugName(const char* name) {
#if EWE_DEBUG_NAMING
		logicalDevice.SetObjectName(pipe, VK_OBJECT_TYPE_PIPELINE, name);
		layout->SetDebugName(name);
#endif
	}
}