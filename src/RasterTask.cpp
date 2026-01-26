#include "EightWinds/RenderGraph/RasterTask.h"

#include "EightWinds/Pipeline/Graphics.h"

#include <unordered_set>

namespace EWE{
	
	RenderTracker::RenderTracker(
		std::string_view name,
		LogicalDevice& logicalDevice,
		Queue& graphicsQueue,
		uint32_t width, uint32_t height,
		std::vector<AttachmentConstructionInfo> const& color_infos,
		AttachmentConstructionInfo depth_info,
		VkRenderingFlags flags
	)
	: full{name, logicalDevice, graphicsQueue, width, height, color_infos, depth_info}
	{
		compact.color_attachments.resize(color_infos.size());
		for (uint8_t i = 0; i < compact.color_attachments.size(); i++) {
			for (uint8_t frame = 0; frame < max_frames_in_flight; frame++) {
				compact.color_attachments[i].imageView[frame] = &full.color_views[i][frame];
			}
			compact.color_attachments[i].info = color_infos[i].info;
		}
		for (uint8_t frame = 0; frame < max_frames_in_flight; frame++) {
			compact.depth_attachment.imageView[frame] = &full.depth_views[frame];
			compact.depth_attachment.info = depth_info.info;
		}
		compact.flags = flags;
		compact.CalculateRenderArea();
	}
	
    void RenderTracker::CascadeFull() {
        compact.Expand(vk_data, vk_info);

    }
	
	
	DeferredPipelineExecute::DeferredPipelineExecute(
		LogicalDevice& logicalDevice, 
		TaskRasterConfig& taskConfig, ObjectRasterData const& rasterData,
		DeferredReference<PipelineParamPack>* pipe_params,
		DeferredReference<ViewportScissorParamPack>* vp_params
	)
		: pipeline{
			new GraphicsPipeline(
				logicalDevice, 0,
				rasterData.layout, 
				taskConfig, rasterData.config, 
				std::vector<VkDynamicState>{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR}
			)
		},
		pipe_paramPack{pipe_params},
		vp_s_paramPack{vp_params}
	{
		
	}

	DeferredPipelineExecute::~DeferredPipelineExecute() {
		delete pipeline;
	}
	void DeferredPipelineExecute::UndeferPipeline(VkViewport const& viewport, VkRect2D const& scissor) {
		for (uint8_t i = 0; i < max_frames_in_flight; i++) {
			pipeline->WriteToParamPack(pipe_paramPack->GetRef(i));
			vp_s_paramPack->GetRef(i).viewport = viewport;
			vp_s_paramPack->GetRef(i).scissor = scissor;
		}
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
		ownsAttachmentLifetime{createAttachments},
		renderTracker{nullptr}
	{
		
	}
	
	
		
	void RasterTask::Record_Vertices(CommandRecord& record) {
		std::unordered_set<ObjectRasterData> unique_configs_vertex{};

		for (auto& [obj_config, _] : vert_draws) unique_configs_vertex.insert(obj_config);
		for (auto& [obj_config, _] : indexed_draws) unique_configs_vertex.insert(obj_config);
		for (auto& [obj_config, _] : vert_draw_counts) unique_configs_vertex.insert(obj_config);
		for (auto& [obj_config, _] : index_draw_counts)	unique_configs_vertex.insert(obj_config);

		for (auto const& obj_config : unique_configs_vertex) {
			auto& vert_ex_back = deferred_pipelines.emplace_back(
				logicalDevice, 
				config, obj_config, 
				record.BindPipeline(), record.SetViewportScissor()
			);

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
	}
	
	void RasterTask::Record_Mesh(CommandRecord& record) {
		std::unordered_set<ObjectRasterData> unique_configs_mesh{};

		for (auto& [obj_config, _] : mesh_draws) unique_configs_mesh.insert(obj_config);
		for (auto& [obj_config, _] : mesh_draw_counts) unique_configs_mesh.insert(obj_config);

		for (auto const& obj_config : unique_configs_mesh) {
			auto& mesh_ex_back = deferred_pipelines.emplace_back(logicalDevice, config, obj_config, record.BindPipeline(), record.SetViewportScissor());

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
	}
	
	void RasterTask::Record(CommandRecord& record) {

		deferred_pipelines.clear();

		Record_Vertices(record);
		Record_Mesh(record);

		//after compiling, go abck thru and write all the pipeline params
	}
	void RasterTask::AdjustPipelines() {
		for (auto& pipe : deferred_pipelines) {
			pipe.UndeferPipeline(viewport, scissor);
		}
	}
}