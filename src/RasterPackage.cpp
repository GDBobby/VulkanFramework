#include "EightWinds/RenderGraph/RasterPackage.h"

#include "EightWinds/Command/InstructionPackage.h"
#include "EightWinds/Pipeline/Graphics.h"

#include <unordered_map>

namespace EWE{

	RasterPackage::RasterPackage(
		std::filesystem::path const& _name,
		LogicalDevice& _logicalDevice,
		Queue& _graphicsQueue,
		TaskRasterConfig const& config
	)
		: Command::InstructionPackage{Command::InstructionPackage::Type::Raster, _name},
		logicalDevice{ _logicalDevice },
		graphicsQueue{ _graphicsQueue },
		task_config{ config },
		attachmentMeta{config.attachment_info.colors.Size() + config.attachment_info.using_depth}
	{
	}

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

	void RasterPackage::Compile() {
		paramPool.Clear();
		if(objectPackages.size() == 0){
			return;
		}

		paramPool.PushBack(Inst::BeginRender);

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
			
			if (latest_pipeline->layout->has_bindless_textures) {
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

		paramPool.PushBack(Inst::EndRender);
	}

	void RasterPackage::Undefer(FullRenderInfo& info){
		//is begin render guaranteed to be first?
		//for_each_frame{
			deferred_vk_render_info = paramPool.param_data.front().CastTo<ParamPack<Inst::BeginRender>>();
			info.Undefer(deferred_vk_render_info);
		//}
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