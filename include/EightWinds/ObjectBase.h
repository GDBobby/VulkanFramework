#pragma once
#include "EightWinds/VulkanHeader.h"

#include "EightWinds/RenderGraph/RasterTask.h"

#include "EightWinds/Buffer.h"

namespace EWE{
	
	struct Model{
		//vertices
		//indices
		//programmable vertex pulling, device buffer addresses, no attributes, no bindings
		
		//i want these buffers to potentially be uninitialized on the GPU. and i want them stack local
		Buffer vertexBuffer;
		Buffer indexBuffer;
		
		uint32_t vertexCount; //if this is 0, then the buffer isn't in usage
		uint32_t indexCount;
		
		const VkIndexType indexType = VK_INDEX_TYPE_UINT16;
		
		bool meshlet = false;
		
		//1 model will be able to create multiple draws (in different pipelines)
		//void deferred_draw = nullptr;
		
		void BindToPush(GlobalPushConstant_Abstract& push, int8_t vertexIndex, int8_t indexIndex){
			assert(!((vertexIndex >= 0 && vertexCount == 0) || (vertexIndex < 0 && vertexCount > 0)));
			assert(!((indexIndex >= 0 && indexCount == 0) || (indexIndex < 0 && indexCount > 0)));
		}
		
		std::string name;
		
		//file location
		[[nodiscard]] explicit Model(LogicalDevice& logicalDevice, std::string_view file_location)
			: name{file_location},
			vertexBuffer{logicalDevice},
			indexBuffer{logicalDevice}
		{
			assert(false && "not supported yet");
		}
		
		/*
			from the construction, I can define the draw function immediately
		*/
		
		template<typename Vertex>
		[[nodiscard]] explicit Model(LogicalDevice& logicalDevice, std::vector<Vertex> const& vertices)
			: vertexBuffer{
				logicalDevice, 
				sizeof(Vertex) * vertices.size(), 1,
				VmaAllocationCreateInfo{
					.flags = static_cast<VmaAllocationCreateFlags>(VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT) | static_cast<VmaAllocationCreateFlags>(VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT)
					.usage = VMA_MEMORY_USAGE_AUTO
				}
			}
		{
			
		}
		template<typename Vertex, std::integral Index>
		[[nodiscard]] explicit Model(LogicalDevice& logicalDevice, std::vector<Vertex> const& vertices, std::vector<Index> const& indices)
			: vertexBuffer{
				logicalDevice,
				sizeof(Vertex) * vertices.size(), 1,
				VmaAllocationCreateInfo{
					.flags = static_cast<VmaAllocationCreateFlags>(VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT) | static_cast<VmaAllocationCreateFlags>(VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT)
					.usage = VMA_MEMORY_USAGE_AUTO
				}
			},
			indexBuffer{
				logicalDevice,
				sizeof(Index) * indices.size(), 1,
				VmaAllocationCreateInfo{
					.flags = static_cast<VmaAllocationCreateFlags>(VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT) | static_cast<VmaAllocationCreateFlags>(VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT)
					.usage = VMA_MEMORY_USAGE_AUTO
				}
			},
			indexType{ std::is_same_v<Index, uint32_t> ? VK_INDEX_TYPE_UINT32, VK_INDEX_TYPE_UINT16}
		{
		}
		
		inline const bool HasVertexBuffer() const {
			return vertexCount >= 0 && vertexBuffer.existsOnTheGPU;
		}
		inline const bool HasIndexBuffer() const {
			return indexCount >= 0 && indexBuffer.existsOnTheGPU;
		}
		
		
		struct DeferredModelDrawParams{
			void* params;
			Model* model; //self reference, so the creator can be referred to
			
			void Undefer(GlobalPushConstant_Abstract gpc, int8_t vertexPushIndex, int8_t indexPushIndex){
			
			if(meshlet){
				assert(vertexCount < 256); //meshproperties.limits.verticesMaximum
				auto reint = reinterpret_cast<DeferredReference<DrawMeshTasksParamPack>*>(params);
				reint->GetRef().x = vertexCount;
				if(vertexPushIndex >= 0 && model->HasVertexBuffer()){
					gpc.buffers[vertexPushIndex] = model->vertexBuffer.GetDeviceAddress();
				}
				if(indexPushIndex >= 0 && model_>HasIndexBuffer()){
					gpc.buffers[indexPushIndex] = model->indexBuffer.GetDeviceAddress();
				}
			}
			else if(model->HasIndexBuffer()){
				
			}
			else if(model->HasVertexBuffer() && indexBufferExists){
				record.DrawIndexed();
			}
			}
		};
		DeferredModelDrawParams Record(CommandRecord& record){
			const bool indexBufferExists = indexCount >= 0 && indexBuffer.existsOnTheGPU;
			
			DeferredModelDrawParams ret;
			ret.model = this;
			
			if(meshlet){
				ret.params = reinterpret_cast<void*>(record.DrawMeshTasks());
			}
			else if(HasIndexBuffer()){
				ret.params = reinterpret_cast<void*>(record.DrawIndexed());
			}
			//else if(HasVertexBuffer()){
			//	record.Draw();
			//}
			else{
				//if there's no vertex buffer, the vert data exists in the shader
				//the only difference is in the push constant (wether or not we take the vertex address)
				ret.params = reinterpret_cast<void*>(record.Draw());
			}
			return ret;
		}
		//negative values for the push index indicate it's not requested
		void WriteToRecord(void* record, int8_t vertexPushIndex, int8_t indexPushIndex){
		}
	};
	
	struct ObjectBase{
		Model model; //could potentially be a dispatch
		ObjectRasterConfig config;
	};
}