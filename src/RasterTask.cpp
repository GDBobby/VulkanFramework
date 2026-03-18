#include "EightWinds/RenderGraph/RasterTask.h"

#include "EightWinds/Pipeline/Graphics.h"

#include <unordered_set>

namespace EWE{

	DeferredPipelineExecute::DeferredPipelineExecute(
		LogicalDevice& logicalDevice, 
		TaskRasterConfig const& taskConfig, ObjectRasterData const& rasterData,
		InstructionPointer<ParamPack::Pipeline>* pipe_params,
		InstructionPointer<ParamPack::Viewport>* vp_params,
		InstructionPointer<ParamPack::Scissor>* sc_params
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
		vp_paramPack{vp_params},
		sc_paramPack{sc_params}
	{
	}
	DeferredPipelineExecute::DeferredPipelineExecute(
		LogicalDevice& logicalDevice,
		TaskRasterConfig const& taskConfig, ObjectRasterData const& rasterData,
		Command::Record& record
	)
		: pipeline{
			new GraphicsPipeline(
				logicalDevice, 0,
				rasterData.layout,
				taskConfig, rasterData.config,
				std::vector<VkDynamicState>{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR}
			)
		},
		pipe_paramPack{ record.Add<Instruction::BindPipeline>()},
		vp_paramPack{ record.Add<Instruction::DS_Viewport>()},
		sc_paramPack{record.Add<Instruction::DS_Scissor>()}
	{
	}


	DeferredPipelineExecute::~DeferredPipelineExecute() {
		if (pipeline != nullptr) {
			delete pipeline;
		}
	}

	DeferredPipelineExecute::DeferredPipelineExecute(DeferredPipelineExecute&& moveSrc) noexcept
		: pipeline{moveSrc.pipeline},
			pipe_paramPack{moveSrc.pipe_paramPack},
			vp_paramPack{moveSrc.vp_paramPack},
			sc_paramPack{moveSrc.sc_paramPack}
	{
		moveSrc.pipeline = nullptr;
		moveSrc.pipe_paramPack = nullptr;
		moveSrc.vp_paramPack = nullptr;
		moveSrc.sc_paramPack = nullptr;
	}

	void DeferredPipelineExecute::UndeferPipeline(VkViewport const& viewport, VkRect2D const& scissor) {
		for (uint8_t i = 0; i < max_frames_in_flight; i++) {
			pipeline->WriteToParamPack(pipe_paramPack->GetRef(i));
			vp_paramPack->GetRef(i).viewport = viewport;
			sc_paramPack->GetRef(i).scissor = scissor;
		}
	}


	RasterTask::RasterTask(
		std::string_view name,
		LogicalDevice& logicalDevice,
		Queue& graphicsQueue,
		TaskRasterConfig const& config,
		FullRenderInfo* renderInfo
	)
		: name{ name },
		logicalDevice{ logicalDevice },
		graphicsQueue{ graphicsQueue },
		config{ config },
		ownsAttachmentLifetime{ renderInfo == nullptr },
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



	void RasterTask::Record_Vertices(Command::Record& record, std::unordered_set<ObjectRasterData>& unique_configs_vertex) {

		std::size_t current_offset = 0;

		for (auto const& obj_config : unique_configs_vertex) {
			auto* pipelineBind = record.Add<Instruction::BindPipeline>();
			auto* vp_bind = record.Add<Instruction::DS_Viewport>();
			auto* sc_bind = record.Add<Instruction::DS_Scissor>();

			deferred_pipelines.ConstructAt(current_offset,
				logicalDevice,
				config, obj_config,
				pipelineBind, vp_bind, sc_bind
			);

			auto& vert_ex_back = deferred_pipelines[current_offset++];

			if (vert_ex_back.pipeline->pipeLayout->descriptorSets.sets.size() > 0) {
				record.Add<Instruction::BindDescriptor>();
			}

			if (vert_draws.Contains(obj_config)) {
				for (auto* draw : vert_draws.at(obj_config).value) {
					if (draw->use_labelPack) {
						draw->deferred_label = record.Add<Instruction::BeginLabel>();
						draw->deferred_push = record.Add<Instruction::Push>();
						draw->paramPack = record.Add<Instruction::Draw>();
						record.Add<Instruction::EndLabel>();
					}
					else {
						draw->deferred_push = record.Add<Instruction::Push>();
						draw->paramPack = record.Add<Instruction::Draw>();
					}
				}
			}

			if (indexed_draws.Contains(obj_config)) {
				for (auto* draw : indexed_draws.at(obj_config).value) {
					if (draw->use_labelPack) {
						draw->deferred_label = record.Add<Instruction::BeginLabel>();
						draw->deferred_push = record.Add<Instruction::Push>();
						draw->paramPack = record.Add<Instruction::DrawIndexed>();
						record.Add<Instruction::EndLabel>();
					}
					else {
						draw->deferred_push = record.Add<Instruction::Push>();
						draw->paramPack = record.Add<Instruction::DrawIndexed>();
					}
				}
			}
			if (vert_draw_counts.Contains(obj_config)) {
				EWE_ASSERT(false, "unsupported");
				//for (auto* draw : vert_draw_counts.at(config).value) {
				//	record.Draw();
				//}
			}

			if (index_draw_counts.Contains(obj_config)) {
				EWE_ASSERT(false, "unsupported");
				//for (auto* draw : index_draw_counts.at(config).value) {
				//	record.DrawIndexed();
				//}
			}


			if (indirect_vert_draws.Contains(obj_config)) {
				for (auto* draw : indirect_vert_draws.at(obj_config).value) {
					if (draw->use_labelPack) {
						draw->deferred_label = record.Add<Instruction::BeginLabel>();
						draw->deferred_push = record.Add<Instruction::Push>();
						draw->paramPack = record.Add<Instruction::DrawIndirect>();
						record.Add<Instruction::EndLabel>();
					}
					else {
						draw->deferred_push = record.Add<Instruction::Push>();
						draw->paramPack = record.Add<Instruction::DrawIndirect>();
					}
				}
			}
			if (indirect_indexed_draws.Contains(obj_config)) {
				for (auto* draw : indirect_indexed_draws.at(obj_config).value) {
					if (draw->use_labelPack) {
						draw->deferred_label = record.Add<Instruction::BeginLabel>();
						draw->deferred_push = record.Add<Instruction::Push>();
						draw->paramPack = record.Add<Instruction::DrawIndexedIndirect>();
						record.Add<Instruction::EndLabel>();
					}
					else {
						draw->deferred_push = record.Add<Instruction::Push>();
						draw->paramPack = record.Add<Instruction::DrawIndexedIndirect>();
					}
				}
			}
			if (indirect_count_vert_draws.Contains(obj_config)) {
				for (auto* draw : indirect_count_vert_draws.at(obj_config).value) {
					if (draw->use_labelPack) {
						draw->deferred_label = record.Add<Instruction::BeginLabel>();
						draw->deferred_push = record.Add<Instruction::Push>();
						draw->paramPack = record.Add<Instruction::DrawIndirectCount>();
						record.Add<Instruction::EndLabel>();
					}
					else {
						draw->deferred_push = record.Add<Instruction::Push>();
						draw->paramPack = record.Add<Instruction::DrawIndirectCount>();
					}
				}
			}
			if (indirect_count_indexed_draws.Contains(obj_config)) {
				for (auto* draw : indirect_count_indexed_draws.at(obj_config).value) {
					if (draw->use_labelPack) {
						draw->deferred_label = record.Add<Instruction::BeginLabel>();
						draw->deferred_push = record.Add<Instruction::Push>();
						draw->paramPack = record.Add<Instruction::DrawIndexedIndirectCount>();
						record.Add<Instruction::EndLabel>();
					}
					else {
						draw->deferred_push = record.Add<Instruction::Push>();
						draw->paramPack = record.Add<Instruction::DrawIndexedIndirectCount>();
					}
				}
			}
		}
	}
	
	void RasterTask::Record_Mesh(Command::Record& record, std::unordered_set<ObjectRasterData>& unique_configs_mesh, std::size_t current_offset) {

		for (auto const& obj_config : unique_configs_mesh) {
			auto* pipelineBind = record.Add<Instruction::BindPipeline>();
			auto* vp_bind = record.Add<Instruction::DS_Viewport>();
			auto* sc_bind = record.Add<Instruction::DS_Scissor>();

			deferred_pipelines.ConstructAt(current_offset, 
				logicalDevice, 
				config, obj_config, 
				pipelineBind, vp_bind, sc_bind
			);

			auto& mesh_ex_back = deferred_pipelines[current_offset++];

			if (mesh_ex_back.pipeline->pipeLayout->descriptorSets.sets.size() > 0) {
				record.Add<Instruction::BindDescriptor>();
			}

			if (mesh_draws.Contains(obj_config)) {
				for (auto* draw : mesh_draws.at(obj_config).value) {
					draw->deferred_push = record.Add<Instruction::Push>();
					draw->paramPack = record.Add<Instruction::DrawMeshTasks>();
				}
			}
			if (mesh_draw_counts.Contains(obj_config)) {
				EWE_ASSERT(false, "unsupported");
				//for (auto* draw : mesh_draw_counts.at(config).value) {
				//	auto& emp = mesh_ex_back.mesh_draws.emplace_back(record.DrawMeshTasks());
				//	draw->paramPack = emp;
				//}
			}
		}
	}
	
	void RasterTask::Record(Command::Record& record, bool labeled) {

		deferred_pipelines.Clear();

		std::vector<VkFormat> formats(config.attachment_set_info.colors.size());
		for (uint32_t i = 0; i < formats.size(); i++) {
			formats[i] = config.attachment_set_info.colors[i].format;
		}
		config.pipelineRenderingCreateInfo.pColorAttachmentFormats = formats.data();
		//^pipelines will be constructed before this goes out of scope
#if EWE_DEBUG_NAMING
		if (labeled) {
			deferred_label = record.Add<Instruction::BeginLabel>();
		}
#endif
		deferred_vk_render_info = record.Add<Instruction::BeginRender>();

		//i need to pre-record unique_configs so that I can resize the deferred_pipeline_execute runtimearray
		std::unordered_set<ObjectRasterData> unique_configs_vertex{};
		for (auto& [obj_config, _] : vert_draws) unique_configs_vertex.insert(obj_config);
		for (auto& [obj_config, _] : indexed_draws) unique_configs_vertex.insert(obj_config);
		for (auto& [obj_config, _] : vert_draw_counts) unique_configs_vertex.insert(obj_config);
		for (auto& [obj_config, _] : index_draw_counts)	unique_configs_vertex.insert(obj_config);

		std::unordered_set<ObjectRasterData> unique_configs_mesh{};
		for (auto& [obj_config, _] : mesh_draws) unique_configs_mesh.insert(obj_config);
		for (auto& [obj_config, _] : mesh_draw_counts) unique_configs_mesh.insert(obj_config);

		deferred_pipelines.Resize(unique_configs_vertex.size() + unique_configs_mesh.size());
		Record_Vertices(record, unique_configs_vertex);
		Record_Mesh(record, unique_configs_mesh, unique_configs_vertex.size());
		record.Add<Instruction::EndRender>();
#if EWE_DEBUG_NAMING
		if (labeled) {
			record.Add<Instruction::EndLabel>();
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


#if EWE_IMGUI
	void RasterTask::Imgui() {
		ImGui::PushID();
		ImGui::Text("Raster Task : %s", name.c_str());
		ImGui::TreeNode("config")
		config.Imgui();
	}
#endif
}