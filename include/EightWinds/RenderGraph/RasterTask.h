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

#include <unordered_set>
#include <optional>

namespace EWE{
	
	struct DrawBase : public GlobalPushConstant_Abstract {
		bool use_labelPack = false;
		DeferredReference<ParamPack::Label>* deferred_label = nullptr;
	};

	template<typename ParamPack>
	struct DrawData : public DrawBase {
		DeferredReference<ParamPack>* paramPack = nullptr;
	};
	
	using VertexDrawData = DrawData<ParamPack::VertexDraw>;
	using IndexedDrawData = DrawData<ParamPack::IndexDraw>;
	using MeshDrawData = DrawData<ParamPack::DrawMeshTasks>;
	using VertexIndirectDrawData = DrawData<ParamPack::DrawIndirect>;
	using IndexedIndirectDrawData = DrawData<ParamPack::DrawIndexedIndirect>;
	using VertexIndirectCountDrawData = DrawData<ParamPack::DrawIndirectCount>;
	using IndexedIndirectCountDrawData = DrawData<ParamPack::DrawIndexedIndirectCount>;

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
		std::vector<ParamPack::VertexDraw> data;
	};
	struct IndexDrawCount : public DrawBase{
		std::vector<ParamPack::IndexDraw> data;
	};
	struct MeshDrawCount : public DrawBase{
		std::vector<ParamPack::DrawMeshTasks> data;
	};

	struct DeferredPipelineExecute {
		Pipeline* pipeline; //needs to be deleted
		//ObjectRasterData rasterData;//i dont really care about keeping the data, besides viewing in debug

		DeferredReference<ParamPack::Pipeline>* pipe_paramPack;
		DeferredReference<ParamPack::ViewportScissor>* vp_s_paramPack;

		[[nodiscard]] explicit DeferredPipelineExecute(
			LogicalDevice& logicalDevice,
			TaskRasterConfig const& taskConfig, ObjectRasterData const& rasterData,
			DeferredReference<ParamPack::Pipeline>* pipe_params,
			DeferredReference<ParamPack::ViewportScissor>* vp_params
		);
		[[nodiscard]] explicit DeferredPipelineExecute(
			LogicalDevice& logicalDevice,
			TaskRasterConfig const& taskConfig, ObjectRasterData const& rasterData,
			Command::Record& record
		);

		~DeferredPipelineExecute();
		DeferredPipelineExecute(DeferredPipelineExecute const& copySrc) = delete;
		DeferredPipelineExecute& operator=(DeferredPipelineExecute&& moveSrc) = delete;
		[[nodiscard]] DeferredPipelineExecute(DeferredPipelineExecute&& moveSrc) noexcept
			: pipeline{moveSrc.pipeline},
				pipe_paramPack{moveSrc.pipe_paramPack},
				vp_s_paramPack{moveSrc.vp_s_paramPack}
		{
			moveSrc.pipeline = nullptr;
			moveSrc.pipe_paramPack = nullptr;
			moveSrc.vp_s_paramPack = nullptr;
		}
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

		TaskRasterConfig config; //temp rename
		//i need a better way to handle dynamic state

		VkViewport viewport; //is viewport x/y going to be permanently tied to attachment width/height?
		VkRect2D scissor;

		FullRenderInfo* renderInfo;
		DeferredReference<VkRenderingInfo>* deferred_vk_render_info{ nullptr };

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
		DrawContainer<VertexDrawCount> vert_draw_counts;
		DrawContainer<IndexDrawCount> index_draw_counts;

		DrawContainer<VertexIndirectDrawData> indirect_vert_draws;
		DrawContainer<IndexedIndirectDrawData> indirect_indexed_draws;
		DrawContainer<VertexIndirectCountDrawData> indirect_count_vert_draws;
		DrawContainer<IndexedIndirectCountDrawData> indirect_count_indexed_draws;

		DrawContainer<MeshDrawData> mesh_draws;
		DrawContainer<MeshDrawCount> mesh_draw_counts;
		
		template<typename T>
		inline void AddHelper(KeyValueContainer<ObjectRasterData, std::vector<T*>>& kv_container, ObjectRasterData const& config, T& draw) {
			if (!kv_container.Contains(config)) {
				kv_container.push_back(config).push_back(&draw);
			}
			else {
				kv_container.at(config).value.push_back(&draw);
			}
		}


		void AddDraw(ObjectRasterData const& config, VertexDrawData& draw) {
			AddHelper(vert_draws, config, draw);
		}
		void AddDraw(ObjectRasterData const& config, IndexedDrawData& draw) {
			AddHelper(indexed_draws, config, draw);
		}
		void AddDraw(ObjectRasterData const& config, MeshDrawData& draw) {
			AddHelper(mesh_draws, config, draw);
		}
		void AddDraw(ObjectRasterData const& config, VertexDrawCount& draw) {
			AddHelper(vert_draw_counts, config, draw);
		}
		void AddDraw(ObjectRasterData const& config, IndexDrawCount& draw) {
			AddHelper(index_draw_counts, config, draw);
		}
		void AddDraw(ObjectRasterData const& config, MeshDrawCount& draw) {
			AddHelper(mesh_draw_counts, config, draw);
		}

		//these are not strongly typed. i need better distinction
		void Add_Vert_IndirectDraw(ObjectRasterData const& config, VertexIndirectDrawData& draw) {
			AddHelper(indirect_vert_draws, config, draw);
		}
		void Add_Indexed_IndirectDraw(ObjectRasterData const& config, IndexedIndirectDrawData& draw) {
			AddHelper(indirect_indexed_draws, config, draw);
		}
		void Add_Vert_IndirectCountDraw(ObjectRasterData const& config, VertexIndirectCountDrawData& draw) {
			AddHelper(indirect_count_vert_draws, config, draw);
		}
		void AddIndexed_IndirectCountDraw(ObjectRasterData const& config, IndexedIndirectCountDrawData& draw) {
			AddHelper(indirect_count_indexed_draws, config, draw);
		}

		//this needs to stay alive as long as these objects are used in a task
		std::vector<DeferredPipelineExecute> deferred_pipelines{};
		DeferredReference<ParamPack::Label>* deferred_label = nullptr;
		
		//the pipelines are unique between vertices and mesh
		void Record_Vertices(Command::Record& record);
		void Record_Mesh(Command::Record& record);

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