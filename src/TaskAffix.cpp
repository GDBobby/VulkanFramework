#include "EightWinds/RenderGraph/TaskAffix.h"

#include "EightWinds/CommandBuffer.h"
#include "EightWinds/Image.h"

namespace EWE{

	BarrierObject::BarrierObject(std::vector<VkImageMemoryBarrier2>&& _imageBarriers, std::vector<VkBufferMemoryBarrier2>&& _bufferBarriers)
	: imageBarriers{ std::forward<std::vector<VkImageMemoryBarrier2>>(_imageBarriers) },
		bufferBarriers{ std::forward<std::vector<VkBufferMemoryBarrier2>>(_bufferBarriers) },
		dependencyInfo{
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			.pNext = nullptr,
			.dependencyFlags = 0,
			.memoryBarrierCount = 0,
			//if this-> isn't used, it might use the forwarded/moved vector, which has had size set to 0
			.bufferMemoryBarrierCount = static_cast<uint32_t>(this->bufferBarriers.size()),
			.pBufferMemoryBarriers = this->bufferBarriers.data(),
			.imageMemoryBarrierCount = static_cast<uint32_t>(this->imageBarriers.size()),
			.pImageMemoryBarriers = this->imageBarriers.data()
		}
	{}
	BarrierObject::BarrierObject()
	: imageBarriers{}, 
		bufferBarriers{}, 
		dependencyInfo{
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			.pNext = nullptr,
			.dependencyFlags = 0,
			.memoryBarrierCount = 0,
			//if this-> isn't used, it might use the forwarded/moved vector, which has had size set to 0
			.bufferMemoryBarrierCount = static_cast<uint32_t>(this->bufferBarriers.size()),
			.pBufferMemoryBarriers = this->bufferBarriers.data(),
			.imageMemoryBarrierCount = static_cast<uint32_t>(this->imageBarriers.size()),
			.pImageMemoryBarriers = this->imageBarriers.data()
	} {

	}
	
	void BarrierObject::FixPointers() {

		dependencyInfo.bufferMemoryBarrierCount = static_cast<uint32_t>(this->bufferBarriers.size());
		dependencyInfo.pBufferMemoryBarriers = this->bufferBarriers.data();
		dependencyInfo.imageMemoryBarrierCount = static_cast<uint32_t>(this->imageBarriers.size());
		dependencyInfo.pImageMemoryBarriers = this->imageBarriers.data();
	}
	
	void TaskAffix::Execute(CommandBuffer& cmdBuf, uint8_t frameIndex){
		auto& barrier = barriers[frameIndex];
		if (!barrier.Empty()) {
			vkCmdPipelineBarrier2(cmdBuf, &(barrier.dependencyInfo));
		}
		for (auto& upd : image_updates) {
			upd.img->data.layout = upd.layout;
		}
	}
}