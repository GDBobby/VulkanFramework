#pragma once

#include "EightWinds/VulkanHeader.h"


#include "EightWinds/RenderGraph/GPUTask.h"
#include "EightWinds/Pipeline/TaskRasterConfig.h"
#include "EightWinds/ObjectRasterConfig.h"

#include "EightWinds/GlobalPushConstant.h"

#include "EightWinds/RenderGraph/Command/Record.h"

#include "EightWinds/Data/KeyValueContainer.h"

#include "EightWinds/Shader.h"
#include "EightWinds/Pipeline/Layout.h"
#include "EightWinds/Pipeline/PipelineBase.h"

#include <unordered_set>

namespace EWE{

	struct RenderTracker {
		RenderInfo vk_data;
		RenderInfo2 compact;
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
	
	
	struct ObjectRasterData{
		//i should do validaiton to ensure this layout is only used with vert draws or mesh draws appropriately
		PipeLayout* layout; //the fragment shader, specifically
		ObjectRasterConfig config;
			
		//the layout is guaranteed to eb unique per engine instance
		//so the address can be used as a direct identifier
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

		DeferredReference<PipelineParamPack>* paramPack;
		DeferredReference<ViewportScissorParamPack>* vp_s_paramPack;

		[[nodiscard]] explicit DeferredPipelineExecute(
			LogicalDevice& logicalDevice, 
			TaskRasterConfig& taskConfig,
			ObjectRasterData const& rasterData,
			DeferredReference<PipelineParamPack>* paramPack
		);
		~DeferredPipelineExecute() {
			delete pipeline;
			//could delete the paramPack too potentially (they're currently a mem leak)
		}
		
		void UndeferPipeline(VkViewport const& viewport, VkRect2D const& scissor) {
			for (uint8_t i = 0; i < max_frames_in_flight; i++) {
				pipeline->WriteToParamPack(paramPack->GetRef(i));
				vp_s_paramPack->GetRef(i).viewport = viewport;
				vp_s_paramPack->GetRef(i).scissor = scissor;
			}
		}
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

		RenderTracker renderTracker;
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
		GPUTask Record();
		
		void AdjustPipelines(){	
			for(auto& pipe : deferred_pipelines){
				pipe.UndeferPipeline(viewport, scissor);
			}
		}
	};
}