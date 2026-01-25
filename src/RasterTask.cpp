#include "EightWinds/RasterTask.h"

#include "EightWinds/Pipeline/Graphics.h"

namespace EWE{
	
	
    void RenderTracker::SetRenderInfo() {
        renderTracker.compact.Expand(&deferred_render_info.GetRef());
    }
	
	
	DeferredPipelineExecute::DeferredPipelineExecute(
		LogicalDevice& logicalDevice, 
		TaskRasterConfig& taskConfig,
		ObjectRasterData const& rasterData,
		DeferredReference<PipelineParamPack>* paramPack,
		DeferredReference<ViewportScissorParamPack>* paramPack;
	)
		: pipeline{
			new GraphicsPipeline(
				logicalDevice, 
				rasterData->layout, 
				taskConfig, rasterData.config, 
				std::vector<VkDynamicState>{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR}
			)
		},
		paramPack{paramPack}
	{
		
	}

	~DeferredPipelineExecute() {
		delete pipeline;
	}
	
	
	RasterTask::RasterTask(
		std::string_view name, 
		LogicalDevice& logicalDevice, 
		Queue& graphicsQueue, 
		TaskRasterConfig const& config, 
		bool createAttachments
	)
		: name{name},
		logicalDevice{logicalDevice},
		graphicsQueue{graphicsQueue},
		config{config},
		ownsAttachmentLifetime{createAttachments}
	{
		
	}
	
	
		
	void RasterTask::Record_Vertices(CommandRecord& record) {
		std::unordered_set<ObjectRasterData>& unique_configs_vertex{};

		for (auto& [obj_config, _] : vert_draws) unique_configs_vertex.insert(obj_config);
		for (auto& [obj_config, _] : indexed_draws) unique_configs_vertex.insert(obj_config);
		for (auto& [obj_config, _] : vert_draw_counts) unique_configs_vertex.insert(obj_config);
		for (auto& [obj_config, _] : index_draw_counts)	unique_configs_vertex.insert(obj_config);

		for (auto const& obj_config : unique_configs_vertex) {
			auto& vert_ex_back = deferred_pipelines.emplace_back(logicalDevice, config, &obj_config, record.BindPipeline(), record.BindViewportScissor());

			if (vert_draws.Contains(obj_config)) {
				for (auto* draw : vert_draws.at(obj_config).value) {
					draw->deferred_push = record.Push();
					draw->paramPack = record.Draw();
				}
			}

			if (indexed_draws.Contains(obj_config)) {
				for (auto* draw : indexed_draws.at(obj_config).value) {
					draw->deferred_push = record.Push();
					draw->paramPack = record.DrawIndexed();
				}
			}
			if (vert_draw_counts.Contains(obj_config)) {
				assert(false && "unsupported");
				//for (auto* draw : vert_draw_counts.at(config).value) {
				//	record.Draw();
				//}
			}

			if (index_draw_counts.Contains(obj_config)) {
				assert(false && "unsupported");
				//for (auto* draw : index_draw_counts.at(config).value) {
				//	record.DrawIndexed();
				//}
			}
		}
		return vertexExecutes;
	}
	
	void RasterTask::Record_Mesh(CommandRecord& record) {
		std::unordered_set<ObjectRasterData>& unique_configs_mesh{};
		std::vector<DeferredPipelineExecute> meshExecutes{};

		for (auto& [obj_config, _] : mesh_draws) unique_configs_mesh.insert(obj_config);
		for (auto& [obj_config, _] : mesh_draw_counts) unique_configs_mesh.insert(obj_config);

		for (auto const& obj_config : unique_configs_mesh) {
			auto& mesh_ex_back = meshExecutes.emplace_back(logicalDevice, config, &obj_config, record.BindPipeline());

			if (mesh_draws.Contains(obj_config)) {
				for (auto* draw : mesh_draws.at(obj_config).value) {
					draw->deferred_push = record.Push();
					draw->paramPack = record.DrawMeshTasks();
				}
			}
			if (mesh_draw_counts.Contains(obj_config)) {
				assert(false && "unsupported");
				//for (auto* draw : mesh_draw_counts.at(config).value) {
				//	auto& emp = mesh_ex_back.mesh_draws.emplace_back(record.DrawMeshTasks());
				//	draw->paramPack = emp;
				//}
			}
		}

		return meshExecutes;
	}
	
	void RasterTask::Record(CommandRecord& record) {

		deferred_pipelines.clear();

		Record_Vertices(record, unique_configs_vertex);
		Record_Mesh(record, unique_configs_mesh);

		//after compiling, go abck thru and write all the pipeline params
	}
	
	GPUTask RasterTask::Record(){
		CommandRecord record{logicalDevice}; //does this need to stay alive past the task creation? i dont think so
		
		Record(record);
		
		GPUTask ret{logicalDevice, graphicsQueue, record, name};
		
		AdjustPipelines();
		return ret;
		
	}
}