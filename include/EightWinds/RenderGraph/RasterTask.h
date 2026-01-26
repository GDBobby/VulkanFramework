#pragma once

#include "EightWinds/VulkanHeader.h"


#include "EightWinds/RenderGraph/GPUTask.h"
#include "EightWinds/RenderGraph/Command/DeferredReference.h"

#include "EightWinds/Pipeline/TaskRasterConfig.h"
#include "EightWinds/ObjectRasterConfig.h"

#include "EightWinds/GlobalPushConstant.h"

#include "EightWinds/RenderGraph/Command/Record.h"

#include "EightWinds/Data/KeyValueContainer.h"

#include "EightWinds/Shader.h"
#include "EightWinds/Pipeline/Layout.h"
#include "EightWinds/Pipeline/PipelineBase.h"

#include "EightWinds/Data/PerFlight.h"

#include "EightWinds/Backend/RenderInfo3.h"

#include <unordered_set>

namespace EWE{


	
	struct RenderTracker {
		RenderInfo3 full;
		RenderInfo2 compact;
		PerFlight<RenderInfo> vk_data;
		PerFlight<VkRenderingInfo> vk_info;
		
		[[nodiscard]] explicit RenderTracker(
			std::string_view name,
			LogicalDevice& logicalDevice,
			Queue& graphicsQueue,
			uint32_t width, uint32_t height,
			std::vector<AttachmentConstructionInfo> const& color_att_info,
			AttachmentConstructionInfo depth_info,
			VkRenderingFlags flags
		);
		
		void CascadeFull();
		
		DeferredReference<VkRenderingInfo> deferred_render_info;
	};
	
	
	using DrawBase = GlobalPushConstant_Abstract;
	
	struct VertexDrawData : public DrawBase {
		DeferredReference<VertexDrawParamPack>* paramPack = nullptr;
	};
	struct IndexedDrawData : public DrawBase {
		DeferredReference<IndexDrawParamPack>* paramPack = nullptr;
	};
	struct MeshDrawData : public DrawBase {
		//if abstract push is empty, we can skip the push
		DeferredReference<DrawMeshTasksParamPack>* paramPack = nullptr;
	};
	

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
	
	//count is for using 1 push constant with multiple objects
	//i dont think that will ever happen?
	struct VertexDrawCount : public DrawBase {
		std::vector<VertexDrawParamPack> data;
	};
	struct IndexDrawCount : public DrawBase{
		std::vector<IndexDrawParamPack> data;	
	};
	struct MeshDrawCount : public DrawBase{
		std::vector<DrawMeshTasksParamPack> data;	
	};

	struct DeferredPipelineExecute {
		Pipeline* pipeline; //needs to be deleted
		//ObjectRasterData rasterData;//i dont really care about keeping the data, besides viewing in debug

		DeferredReference<PipelineParamPack>* pipe_paramPack;
		DeferredReference<ViewportScissorParamPack>* vp_s_paramPack;

		[[nodiscard]] explicit DeferredPipelineExecute(
			LogicalDevice& logicalDevice,
			TaskRasterConfig& taskConfig, ObjectRasterData const& rasterData,
			DeferredReference<PipelineParamPack>* pipe_params,
			DeferredReference<ViewportScissorParamPack>* vp_params
		);
		~DeferredPipelineExecute();
		
		void UndeferPipeline(VkViewport const& viewport, VkRect2D const& scissor);
	};

	//a single Raster task will use one set of attachments (no swapping)
	//it can potentially read from resources
	//a mesh shader can write to resources I believe
	//what about shaders????
	struct RasterTask{
		const std::string name;
		LogicalDevice& logicalDevice;
		Queue& graphicsQueue;

		TaskRasterConfig config; //temp rename
		//i need a better way to handle dynamic state

		VkViewport viewport;
		VkRect2D scissor;

		//i need to figure out the attachments as well
		//this object should be capable of constructing the attachments, based on the config, 
		//or borrow the configs with asserts (based on the config)
		std::vector<Image> color_attachments{};
		std::vector<Image> depth_attachments{};

		RenderTracker* renderTracker;
		bool ownsAttachmentLifetime = true;

		[[nodiscard]] explicit RasterTask(std::string_view name, LogicalDevice& logicalDevice, Queue& graphicsQueue, TaskRasterConfig const& config, bool createAttachments);
		
		//fully polymorhpism, with dynamic dispatch. woudl need a virtual Draw() = 0;
		//KeyValueContainer<ObjectRasterConfig, std::vector<DrawBase*>> draws;
		//if i made it so that the user is in charge of condensing pipelines, i wouldnt' have to store these
		KeyValueContainer<ObjectRasterData, std::vector<VertexDrawData*>> vert_draws;
		KeyValueContainer<ObjectRasterData, std::vector<IndexedDrawData*>> indexed_draws;
		KeyValueContainer<ObjectRasterData, std::vector<MeshDrawData*>> mesh_draws;
		
		KeyValueContainer<ObjectRasterData, std::vector<VertexDrawCount*>> vert_draw_counts;
		KeyValueContainer<ObjectRasterData, std::vector<IndexDrawCount*>> index_draw_counts;
		KeyValueContainer<ObjectRasterData, std::vector<MeshDrawCount*>> mesh_draw_counts;
		
		void AddDraw(ObjectRasterData const& config, VertexDrawData& draw) {
			vert_draws.at(config).value.push_back(&draw);
		}
		void AddDraw(ObjectRasterData const& config, IndexedDrawData& draw) {
			indexed_draws.at(config).value.push_back(&draw);
		}
		void AddDraw(ObjectRasterData const& config, MeshDrawData& draw) {
			mesh_draws.at(config).value.push_back(&draw);
		}
		void AddDraw(ObjectRasterData const& config, VertexDrawCount& draw) {
			vert_draw_counts.at(config).value.push_back(&draw);
		}
		void AddDraw(ObjectRasterData const& config, IndexDrawCount& draw) {
			index_draw_counts.at(config).value.push_back(&draw);
		}
		void AddDraw(ObjectRasterData const& config, MeshDrawCount& draw) {
			mesh_draw_counts.at(config).value.push_back(&draw);
		}

		//this needs to stay alive as long as these objects are used in a task
		std::vector<DeferredPipelineExecute> deferred_pipelines{}; 
		
		//the pipelines are unique between vertices and mesh
		void Record_Vertices(CommandRecord& record);
		void Record_Mesh(CommandRecord& record);

		void Record(CommandRecord& record);
		
		void AdjustPipelines();
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