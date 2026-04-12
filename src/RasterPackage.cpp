#include "EightWinds/RenderGraph/RasterPackage.h"

#include "EightWinds/Pipeline/Graphics.h"

#include <unordered_map>

namespace EWE{

	/*
	DeferredPipelineExecute::DeferredPipelineExecute(
		LogicalDevice& logicalDevice, 
		TaskRasterConfig const& taskConfig, ObjectRasterData const& rasterData,
		InstructionPointer<ParamPack<Inst::BindPipeline>>* pipe_params,
		InstructionPointer<ParamPack<Inst::DS_Viewport>>* vp_params,
		InstructionPointer<ParamPack<Inst::DS_Scissor>>* sc_params
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
		pipe_paramPack{ record.Add<Inst::BindPipeline>()},
		vp_paramPack{ record.Add<Inst::DS_Viewport>()},
		sc_paramPack{record.Add<Inst::DS_Scissor>()}
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
	*/

	RasterPackage::RasterPackage(
		std::string_view _name,
		LogicalDevice& _logicalDevice,
		Queue& _graphicsQueue,
		TaskRasterConfig const& config,
		FullRenderInfo* _renderInfo
	)
		: name{ _name },
		logicalDevice{ _logicalDevice },
		graphicsQueue{ _graphicsQueue },
		task_config{ config },
		renderInfo{ _renderInfo },
		ownsAttachmentLifetime{ renderInfo == nullptr }
	{
		if (renderInfo == nullptr) {
			renderInfo = new FullRenderInfo(
				_name,
				logicalDevice, graphicsQueue,
				config.attachment_set_info
			);
		}
	}


/*
	void RasterPackage::Record_Vertices(Command::Record& record, std::unordered_set<ObjectRasterData>& unique_configs_vertex) {

		std::size_t current_offset = 0;

		for (auto const& obj_config : unique_configs_vertex) {
			auto* pipelineBind = record.Add<Inst::BindPipeline>();
			auto* vp_bind = record.Add<Inst::DS_Viewport>();
			auto* sc_bind = record.Add<Inst::DS_Scissor>();

			deferred_pipelines.ConstructAt(current_offset,
				logicalDevice,
				task_config, obj_config,
				pipelineBind, vp_bind, sc_bind
			);

			auto& vert_ex_back = deferred_pipelines[current_offset++];

			if (vert_ex_back.pipeline->pipeLayout->descriptorSets.sets.size() > 0) {
				record.Add<Inst::BindDescriptor>();
			}

			if (vert_draws.Contains(obj_config)) {
				for (auto* draw : vert_draws.at(obj_config).value) {
					if (draw->use_labelPack) {
						draw->deferred_label = record.Add<Inst::BeginLabel>();
						draw->deferred_push = record.Add<Inst::Push>();
						draw->paramPack = record.Add<Inst::Draw>();
						record.Add<Inst::EndLabel>();
					}
					else {
						draw->deferred_push = record.Add<Inst::Push>();
						draw->paramPack = record.Add<Inst::Draw>();
					}
				}
			}

			if (indexed_draws.Contains(obj_config)) {
				for (auto* draw : indexed_draws.at(obj_config).value) {
					if (draw->use_labelPack) {
						draw->deferred_label = record.Add<Inst::BeginLabel>();
						draw->deferred_push = record.Add<Inst::Push>();
						draw->paramPack = record.Add<Inst::DrawIndexed>();
						record.Add<Inst::EndLabel>();
					}
					else {
						draw->deferred_push = record.Add<Inst::Push>();
						draw->paramPack = record.Add<Inst::DrawIndexed>();
					}
				}
			}


			if (indirect_vert_draws.Contains(obj_config)) {
				for (auto* draw : indirect_vert_draws.at(obj_config).value) {
					if (draw->use_labelPack) {
						draw->deferred_label = record.Add<Inst::BeginLabel>();
						draw->deferred_push = record.Add<Inst::Push>();
						draw->paramPack = record.Add<Inst::DrawIndirect>();
						record.Add<Inst::EndLabel>();
					}
					else {
						draw->deferred_push = record.Add<Inst::Push>();
						draw->paramPack = record.Add<Inst::DrawIndirect>();
					}
				}
			}
			if (indirect_indexed_draws.Contains(obj_config)) {
				for (auto* draw : indirect_indexed_draws.at(obj_config).value) {
					if (draw->use_labelPack) {
						draw->deferred_label = record.Add<Inst::BeginLabel>();
						draw->deferred_push = record.Add<Inst::Push>();
						draw->paramPack = record.Add<Inst::DrawIndexedIndirect>();
						record.Add<Inst::EndLabel>();
					}
					else {
						draw->deferred_push = record.Add<Inst::Push>();
						draw->paramPack = record.Add<Inst::DrawIndexedIndirect>();
					}
				}
			}
			if (indirect_count_vert_draws.Contains(obj_config)) {
				for (auto* draw : indirect_count_vert_draws.at(obj_config).value) {
					if (draw->use_labelPack) {
						draw->deferred_label = record.Add<Inst::BeginLabel>();
						draw->deferred_push = record.Add<Inst::Push>();
						draw->paramPack = record.Add<Inst::DrawIndirectCount>();
						record.Add<Inst::EndLabel>();
					}
					else {
						draw->deferred_push = record.Add<Inst::Push>();
						draw->paramPack = record.Add<Inst::DrawIndirectCount>();
					}
				}
			}
			if (indirect_count_indexed_draws.Contains(obj_config)) {
				for (auto* draw : indirect_count_indexed_draws.at(obj_config).value) {
					if (draw->use_labelPack) {
						draw->deferred_label = record.Add<Inst::BeginLabel>();
						draw->deferred_push = record.Add<Inst::Push>();
						draw->paramPack = record.Add<Inst::DrawIndexedIndirectCount>();
						record.Add<Inst::EndLabel>();
					}
					else {
						draw->deferred_push = record.Add<Inst::Push>();
						draw->paramPack = record.Add<Inst::DrawIndexedIndirectCount>();
					}
				}
			}
		}
	}
	
	void RasterPackage::Record_Mesh(Command::Record& record, std::unordered_set<ObjectRasterData>& unique_configs_mesh, std::size_t current_offset) {

		for (auto const& obj_config : unique_configs_mesh) {
			auto* pipelineBind = record.Add<Inst::BindPipeline>();
			auto* vp_bind = record.Add<Inst::DS_Viewport>();
			auto* sc_bind = record.Add<Inst::DS_Scissor>();

			deferred_pipelines.ConstructAt(current_offset, 
				logicalDevice, 
				task_config, obj_config, 
				pipelineBind, vp_bind, sc_bind
			);

			auto& mesh_ex_back = deferred_pipelines[current_offset++];

			if (mesh_ex_back.pipeline->pipeLayout->descriptorSets.sets.size() > 0) {
				record.Add<Inst::BindDescriptor>();
			}

			if (mesh_draws.Contains(obj_config)) {
				for (auto* draw : mesh_draws.at(obj_config).value) {
					draw->deferred_push = record.Add<Inst::Push>();
					draw->paramPack = record.Add<Inst::DrawMeshTasks>();
				}
			}
		}
	}
	
	void RasterPackage::Record(Command::Record& record, bool labeled) {

		deferred_pipelines.Clear();

		std::vector<VkFormat> formats(task_config.attachment_set_info.colors.size());
		for (uint32_t i = 0; i < formats.size(); i++) {
			formats[i] = task_config.attachment_set_info.colors[i].format;
		}
		task_config.pipelineRenderingCreateInfo.pColorAttachmentFormats = formats.data();
		//^pipelines will be constructed before this goes out of scope
#if EWE_DEBUG_NAMING
		if (labeled) {
			deferred_label = record.Add<Inst::BeginLabel>();
		}
#endif
		deferred_vk_render_info = record.Add<Inst::BeginRender>();

		//i need to pre-record unique_configs so that I can resize the deferred_pipeline_execute runtimearray
		std::unordered_set<ObjectRasterData> unique_configs_vertex{};
		for (auto& [obj_config, _] : vert_draws) unique_configs_vertex.insert(obj_config);
		for (auto& [obj_config, _] : indexed_draws) unique_configs_vertex.insert(obj_config);
		//for (auto& [obj_config, _] : vert_draw_counts) unique_configs_vertex.insert(obj_config);
		//for (auto& [obj_config, _] : index_draw_counts)	unique_configs_vertex.insert(obj_config);

		std::unordered_set<ObjectRasterData> unique_configs_mesh{};
		for (auto& [obj_config, _] : mesh_draws) unique_configs_mesh.insert(obj_config);
		//for (auto& [obj_config, _] : mesh_draw_counts) unique_configs_mesh.insert(obj_config);

		deferred_pipelines.Resize(unique_configs_vertex.size() + unique_configs_mesh.size());
		Record_Vertices(record, unique_configs_vertex);
		Record_Mesh(record, unique_configs_mesh, unique_configs_vertex.size());
		record.Add<Inst::EndRender>();
#if EWE_DEBUG_NAMING
		if (labeled) {
			record.Add<Inst::EndLabel>();
		}
#endif

		//after compiling, go abck thru and write all the pipeline params
	}
	void RasterPackage::AdjustPipelines() {
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
		*/

	void AssignToPipeline(LogicalDevice& logicalDevice, std::unordered_map<ObjectRasterData, std::vector<Command::ObjectPackage*>>& pipeline_group, Command::ObjectPackage* pkg){

		ObjectRasterData objRasterData{
			.layout = PipeLayout::GetLayout(logicalDevice, pkg->payload.shaders),
			.config = pkg->payload.config
		};
		auto found = pipeline_group.find(objRasterData);
		if(found == pipeline_group.end()){
			bool result = pipeline_group.try_emplace(objRasterData, std::vector<Command::ObjectPackage*>{pkg}).second;
			EWE_ASSERT(result, "failed to emplace into pipelien gorup");
		}
		else{
			found->second.push_back(pkg);
		}
	}

	void RasterPackage::Compile(){

		paramPool.Clear();

		if(objectPackages.size() > 0){
			paramPool.PushBack(Inst::BeginRender);
		}

		std::unordered_map<ObjectRasterData, std::vector<Command::ObjectPackage*>> vertex_pipeline_groups;
		std::unordered_map<ObjectRasterData, std::vector<Command::ObjectPackage*>> mesh_pipeline_groups;

		for(auto& pkg : objectPackages){
			if(pkg->GetDrawType() == Command::ObjectPackage::DrawType::Vertex){
				AssignToPipeline(logicalDevice, vertex_pipeline_groups, pkg);
			}
			else if(pkg->GetDrawType() == Command::ObjectPackage::DrawType::Mesh){
				AssignToPipeline(logicalDevice, mesh_pipeline_groups, pkg);
			}
		}

		for(auto& group : vertex_pipeline_groups){
			auto latest_pipeline = new GraphicsPipeline(
				logicalDevice, 0,
				group.first.layout, 
				task_config, group.first.config,
				std::vector<VkDynamicState>{VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_VIEWPORT} 
			);
			created_pipelines.push_back(latest_pipeline);
			
			paramPool.PushBack(
				ParamPack<Inst::BindPipeline>{
					.pipe = latest_pipeline->vkPipe,
					.layout = latest_pipeline->layout->vkLayout,
					.bindPoint = latest_pipeline->layout->bindPoint
				}
			);
			
			if (latest_pipeline->layout->descriptorSets.sets.size() > 0) {
				paramPool.PushBack(Inst::BindDescriptor);
			}
				
			paramPool.PushBack(
				ParamPack<Inst::DS_Viewport>{
					.viewport = viewport
				}
			);
			
			paramPool.PushBack(
				ParamPack<Inst::DS_Scissor>{
					.scissor = scissor
				}
			);

			for(auto& obj : group.second){
				paramPool.PushBack(
					ParamPack<Inst::Ext_Pool>{
						.pool = &obj->paramPool
					}
				);
			}
		}

		if(objectPackages.size() > 0){
			paramPool.PushBack(Inst::EndRender);
			deferred_vk_render_info = reinterpret_cast<InstructionPointer<VkRenderingInfo>*>(&paramPool.param_data[0]);
			renderInfo->Undefer(deferred_vk_render_info);
		}

	}


#if EWE_IMGUI
	void RasterPackage::Imgui() {
		ImGui::PushID();
		ImGui::Text("Raster Task : %s", name.c_str());
		ImGui::TreeNode("config")
		config.Imgui();
	}
#endif
}