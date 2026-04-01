#pragma once

#include "EightWinds/Command/InstructionType.h"
#include "EightWinds/Command/ParamPacks.h"
#include "EightWinds/VulkanHeader.h"


#include "EightWinds/RenderGraph/GPUTask.h"
#include "EightWinds/Command/InstructionPointer.h"

#include "EightWinds/Pipeline/TaskRasterConfig.h"
#include "EightWinds/ObjectRasterConfig.h"

#include "EightWinds/GlobalPushConstant.h"

#include "EightWinds/Command/Record.h"

#include "EightWinds/Data/KeyValueContainer.h"

#include "EightWinds/Pipeline/Layout.h"
#include "EightWinds/Pipeline/PipelineBase.h"


#include <unordered_set>

namespace EWE{

	
	struct DrawBase : public GlobalPushConstant_Abstract {
		bool use_labelPack = false;
		InstructionPointer<ParamPack<Inst::BeginLabel>>* deferred_label = nullptr;
	};

	template<Inst::Type IType>
	struct DrawData : public DrawBase {
		InstructionPointer<ParamPack<IType>>* paramPack = nullptr;
	};
	
	using VertexDrawData = DrawData<Inst::Draw>;
	using IndexedDrawData = DrawData<Inst::DrawIndexed>;
	using MeshDrawData = DrawData<Inst::DrawMeshTasks>;
	using VertexIndirectDrawData = DrawData<Inst::DrawIndirect>;
	using IndexedIndirectDrawData = DrawData<Inst::DrawIndexedIndirect>;
	using MeshIndirectDrawData = DrawData<Inst::DrawMeshTasksIndirect>;
	using VertexIndirectCountDrawData = DrawData<Inst::DrawIndirectCount>;
	using IndexedIndirectCountDrawData = DrawData<Inst::DrawIndexedIndirectCount>;
	using MeshIndirectCountDrawData = DrawData<Inst::DrawMeshTasksIndirectCount>;

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
	template<Inst::Type IType>
	struct DrawDataCount : public DrawBase {
		std::vector<ParamPack<IType>> data;
	};

	struct DeferredPipelineExecute {
		Pipeline* pipeline; //needs to be deleted
		//ObjectRasterData rasterData;//i dont really care about keeping the data, besides viewing in debug

		InstructionPointer<ParamPack<Inst::BindPipeline>>* pipe_paramPack;
		InstructionPointer<ParamPack<Inst::DS_Viewport>>* vp_paramPack;
		InstructionPointer<ParamPack<Inst::DS_Scissor>>* sc_paramPack;

		[[nodiscard]] explicit DeferredPipelineExecute(
			LogicalDevice& logicalDevice,
			TaskRasterConfig const& taskConfig, ObjectRasterData const& rasterData,
			InstructionPointer<ParamPack<Inst::BindPipeline>>* pipe_params,
			InstructionPointer<ParamPack<Inst::DS_Viewport>>* vp_params,
			InstructionPointer<ParamPack<Inst::DS_Scissor>>* sc_params
		);
		[[nodiscard]] explicit DeferredPipelineExecute(
			LogicalDevice& logicalDevice,
			TaskRasterConfig const& taskConfig, ObjectRasterData const& rasterData,
			Command::Record& record
		);

		~DeferredPipelineExecute();
		DeferredPipelineExecute(DeferredPipelineExecute const& copySrc) = delete;
		DeferredPipelineExecute& operator=(DeferredPipelineExecute&& moveSrc) = delete;
		[[nodiscard]] DeferredPipelineExecute(DeferredPipelineExecute&& moveSrc) noexcept;
		DeferredPipelineExecute& operator=(DeferredPipelineExecute const& copySrc) = delete;
		
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

		TaskRasterConfig task_config; //temp rename
		//i need a better way to handle dynamic state

		VkViewport viewport; //is viewport x/y going to be permanently tied to attachment width/height?
		VkRect2D scissor;

		FullRenderInfo* renderInfo;
		InstructionPointer<VkRenderingInfo>* deferred_vk_render_info{ nullptr };

		bool ownsAttachmentLifetime = true;

		[[nodiscard]] explicit RasterTask(std::string_view name, LogicalDevice& logicalDevice, Queue& graphicsQueue, TaskRasterConfig const& config, FullRenderInfo* renderInfo);
		
		//fully polymorhpism, with dynamic dispatch. woudl need a virtual Draw() = 0;
		//KeyValueContainer<ObjectRasterConfig, std::vector<DrawBase*>> draws;
		//if i made it so that the user is in charge of condensing pipelines, i wouldnt' have to store these
		
		//i could template these to make it MUCH more convenient/clean but idk if its worth the effort
		template<typename DrawT>
		using DrawContainer = KeyValueContainer<ObjectRasterData, std::vector<DrawT*>>;

		DrawContainer<VertexDrawData> vert_draws;
		DrawContainer<IndexedDrawData> indexed_draws;
		//DrawContainer<DrawCount> vert_draw_counts;
		//DrawContainer<IndexDrawCount> index_draw_counts;

		DrawContainer<VertexIndirectDrawData> indirect_vert_draws;
		DrawContainer<IndexedIndirectDrawData> indirect_indexed_draws;
		DrawContainer<VertexIndirectCountDrawData> indirect_count_vert_draws;
		DrawContainer<IndexedIndirectCountDrawData> indirect_count_indexed_draws;

		DrawContainer<MeshDrawData> mesh_draws;
		//DrawContainer<MeshDrawCount> mesh_draw_counts;
		
		template<typename T>
		inline void AddHelper(KeyValueContainer<ObjectRasterData, std::vector<T*>>& kv_container, ObjectRasterData const& config, T* draw) {
			if (!kv_container.Contains(config)) {
				kv_container.push_back(config).push_back(draw);
			}
			else {
				kv_container.at(config).value.push_back(draw);
			}
		}


		void AddDraw(ObjectRasterData const& config, VertexDrawData* draw) {
			AddHelper(vert_draws, config, draw);
		}
		void AddDraw(ObjectRasterData const& config, IndexedDrawData* draw) {
			AddHelper(indexed_draws, config, draw);
		}
		void AddDraw(ObjectRasterData const& config, MeshDrawData* draw) {
			AddHelper(mesh_draws, config, draw);
		}

		//these are not strongly typed. i need better distinction
		void Add_Vert_IndirectDraw(ObjectRasterData const& config, VertexIndirectDrawData* draw) {
			AddHelper(indirect_vert_draws, config, draw);
		}
		void Add_Indexed_IndirectDraw(ObjectRasterData const& config, IndexedIndirectDrawData* draw) {
			AddHelper(indirect_indexed_draws, config, draw);
		}
		void Add_Vert_IndirectCountDraw(ObjectRasterData const& config, VertexIndirectCountDrawData* draw) {
			AddHelper(indirect_count_vert_draws, config, draw);
		}
		void AddIndexed_IndirectCountDraw(ObjectRasterData const& config, IndexedIndirectCountDrawData* draw) {
			AddHelper(indirect_count_indexed_draws, config, draw);
		}

		//this needs to stay alive as long as these objects are used in a task
		HeapBlock<DeferredPipelineExecute> deferred_pipelines{};
		InstructionPointer<ParamPack<Inst::BeginLabel>>* deferred_label = nullptr;
		
		//the pipelines are unique between vertices and mesh
		void Record_Vertices(Command::Record& record, std::unordered_set<ObjectRasterData>& unique_configs);
		void Record_Mesh(Command::Record& record, std::unordered_set<ObjectRasterData>& unique_configs, std::size_t current_offset);

		void Record(Command::Record& record, bool labeled = false);
		
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