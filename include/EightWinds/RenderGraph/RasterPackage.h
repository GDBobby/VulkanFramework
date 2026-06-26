#pragma once

#include "EightWinds/VulkanHeader.h"

#include "EightWinds/RenderGraph/GPUTask.h"

#include "EightWinds/ObjectRasterConfig.h"

#include "EightWinds/Data/KeyValueContainer.h"

#include "EightWinds/Pipeline/TaskRasterConfig.h"
#include "EightWinds/Pipeline/Layout.h"
#include "EightWinds/Pipeline/PipelineBase.h"

#include "EightWinds/Command/ObjectInstructionPackage.h"
#include "EightWinds/Command/InstructionPointer.h"
#include "EightWinds/Command/InstructionType.h"
#include "EightWinds/Command/ParamPacks.h"
#include "EightWinds/Command/ParamPool.h"
#include "EightWinds/Command/InstructionPackage.h"

#include <unordered_set>

namespace EWE{

	//i should do validaiton to ensure this layout is only used with vert draws or mesh draws appropriately
	struct ObjectRasterData{
		//the layout is guaranteed to eb unique per engine instance
		//so the address can be used as a direct identifier
		PipeLayout* layout;
		ObjectRasterConfig config;
			
		bool operator==(ObjectRasterData const& other) const {
			return layout == other.layout && config == other.config;
		}

	};

	//a single Raster task will use one set of attachments (no swapping)
	//it can potentially read from resources
	//a mesh shader can write to resources I believe
	//what about shaders????
	struct RasterPackage : Command::InstructionPackage{
		LogicalDevice& logicalDevice;
		Queue& graphicsQueue;

		TaskRasterConfig task_config; //temp rename
		//i need a better way to handle dynamic state

		VkViewport viewport; //is viewport x/y going to be permanently tied to attachment width/height?
		VkRect2D scissor;

		//attachmentmeta along iwth deferred_vk_render_info, combined with attachmentsetinfo in task_config, will be used to deduce fullrenderinfo
		RuntimeArray<AttachmentMeta> attachmentMeta; //
		InstructionPointer<ParamPack<Inst::BeginRender>>* deferred_vk_render_info{ nullptr };

		[[nodiscard]] explicit RasterPackage(
			std::filesystem::path const& name,
			LogicalDevice& logicalDevice, Queue& graphicsQueue, 
			TaskRasterConfig const& config
		);
		
		std::vector<Command::ObjectPackage*> objectPackages;

		std::vector<Pipeline*> created_pipelines; //tracked for deconstruction
		
		void Compile();

		//i dont need this yet, currently im using Inst::ExtPool to prevent copying/moving the data
		//std::vector<ParamPointerChain> CompileAdjustPPCs(std::vector<ParamPointerChain> const& unadjusted);

		void AdjustPipelines();

		void Undefer(FullRenderInfo& info);
	};
} //namespace EWE


template<>
struct std::hash<EWE::ObjectRasterData> {
	size_t operator()(EWE::ObjectRasterData const& d) const noexcept {
		EWE::HashCombiner hashCombiner{};
		hashCombiner.Combine(std::hash<std::size_t>{}(reinterpret_cast<std::size_t>(d.layout)));
		hashCombiner.Combine(std::hash<EWE::ObjectRasterConfig>{}(d.config));
		return hashCombiner.seed;
	}
};