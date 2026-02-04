#include "EightWinds/RenderGraph/RasterTask.h"

#include "EightWinds/Pipeline/Graphics.h"

#include <unordered_set>

namespace EWE{

	DeferredPipelineExecute::DeferredPipelineExecute(
		LogicalDevice& logicalDevice, 
		TaskRasterConfig const& taskConfig, ObjectRasterData const& rasterData,
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
	DeferredPipelineExecute::DeferredPipelineExecute(
		LogicalDevice& logicalDevice,
		TaskRasterConfig const& taskConfig, ObjectRasterData const& rasterData,
		CommandRecord& record
	)
		: pipeline{
			new GraphicsPipeline(
				logicalDevice, 0,
				rasterData.layout,
				taskConfig, rasterData.config,
				std::vector<VkDynamicState>{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR}
			)
		},
		pipe_paramPack{ record.BindPipeline() },
		vp_s_paramPack{ record.SetViewportScissor() }
	{
	}


	DeferredPipelineExecute::~DeferredPipelineExecute() {
		if (pipeline != nullptr) {
			delete pipeline;
		}
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
		FullRenderInfo* renderInfo
	)
		: name{name},
		logicalDevice{logicalDevice},
		graphicsQueue{graphicsQueue},
		config{config},
		ownsAttachmentLifetime{renderInfo == nullptr},
		renderInfo{ renderInfo }
	{
		if (renderInfo == nullptr) {
			renderInfo = new FullRenderInfo(
				name,
				logicalDevice, graphicsQueue,
				config.attachment_set_info
			);
		}
	}
	
	
		
	void RasterTask::Record_Vertices(CommandRecord& record) {
		std::unordered_set<ObjectRasterData> unique_configs_vertex{};

		for (auto& [obj_config, _] : vert_draws) unique_configs_vertex.insert(obj_config);
		for (auto& [obj_config, _] : indexed_draws) unique_configs_vertex.insert(obj_config);
		for (auto& [obj_config, _] : vert_draw_counts) unique_configs_vertex.insert(obj_config);
		for (auto& [obj_config, _] : index_draw_counts)	unique_configs_vertex.insert(obj_config);

		for (auto const& obj_config : unique_configs_vertex) {
			auto* pipelineBind = record.BindPipeline();
			auto* vpBind = record.SetViewportScissor();
			auto& vert_ex_back = deferred_pipelines.emplace_back(
				logicalDevice, 
				config, obj_config, 
				pipelineBind, vpBind
			);
			if (vert_ex_back.pipeline->pipeLayout->descriptorSets.sets.size() > 0) {
				record.BindDescriptor();
			}

			if (vert_draws.Contains(obj_config)) {
				for (auto* draw : vert_draws.at(obj_config).value) {
					if (draw->use_labelPack) {
						draw->deferred_label = record.BeginLabel();
						draw->deferred_push = record.Push();
						draw->paramPack = record.Draw();
						record.EndLabel();
					}
					else {
						draw->deferred_push = record.Push();
						draw->paramPack = record.Draw();
					}
				}
			}

			if (indexed_draws.Contains(obj_config)) {
				for (auto* draw : indexed_draws.at(obj_config).value) {
					if (draw->use_labelPack) {
						draw->deferred_label = record.BeginLabel();
						draw->deferred_push = record.Push();
						draw->paramPack = record.DrawIndexed();
						record.EndLabel();
					}
					else {
						draw->deferred_push = record.Push();
						draw->paramPack = record.DrawIndexed();
					}
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
			auto* pipelineBind = record.BindPipeline();
			auto* vpBind = record.SetViewportScissor();
			auto& mesh_ex_back = deferred_pipelines.emplace_back(
				logicalDevice, 
				config, obj_config, 
				pipelineBind, vpBind
			);
			if (mesh_ex_back.pipeline->pipeLayout->descriptorSets.sets.size() > 0) {
				record.BindDescriptor();
			}

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
	
	void RasterTask::Record(CommandRecord& record, bool labeled) {

		deferred_pipelines.clear();

		std::vector<VkFormat> formats(config.attachment_set_info.colors.size());
		for (uint32_t i = 0; i < formats.size(); i++) {
			formats[i] = config.attachment_set_info.colors[i].format;
		}
		config.pipelineRenderingCreateInfo.pColorAttachmentFormats = formats.data();
		//^pipelines will be constructed before this goes out of scope
#if EWE_DEBUG_NAMING
		if (labeled) {
			deferred_label = record.BeginLabel();
		}
#endif
		deferred_vk_render_info = record.BeginRender();
		Record_Vertices(record);
		Record_Mesh(record);
		record.EndRender();
#if EWE_DEBUG_NAMING
		if (labeled) {
			record.EndLabel();
		}
#endif

		//after compiling, go abck thru and write all the pipeline params
	}
	void RasterTask::AdjustPipelines() {
		renderInfo->Undefer(deferred_vk_render_info);

		for (auto& pipe : deferred_pipelines) {
			pipe.UndeferPipeline(viewport, scissor);
		}
		if (deferred_label != nullptr) {
			for (uint8_t i = 0; i < max_frames_in_flight; i++) {
				deferred_label->GetRef(i).name = name.c_str();
				deferred_label->GetRef(i).red = 1.f;
				deferred_label->GetRef(i).green = 0.f;
				deferred_label->GetRef(i).blue = 0.f;
			}
		}
	}
}