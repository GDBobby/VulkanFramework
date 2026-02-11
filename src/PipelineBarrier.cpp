#include "EightWinds/Backend/PipelineBarrier.h"

#include "EightWinds/Queue.h"
#include "EightWinds/Buffer.h"
#include "EightWinds/Image.h"

#include <iterator>

namespace EWE {
    PipelineBarrier::PipelineBarrier() :
        memoryBarriers{},
        imageBarriers{},
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
		}
    {}
	
	PipelineBarrier::PipelineBarrier(
		std::vector<VkImageMemoryBarrier2>&& imageBarriers
	)
	: imageBarriers{ std::forward<std::vector<VkImageMemoryBarrier2>>(imageBarriers) },
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
		}
	{}
	PipelineBarrier::PipelineBarrier(
		std::vector<VkImageMemoryBarrier2>&& imageBarriers, 
		std::vector<VkBufferMemoryBarrier2>&& bufferBarriers
	)
	: imageBarriers{ std::forward<std::vector<VkImageMemoryBarrier2>>(imageBarriers) },
		bufferBarriers{ std::forward<std::vector<VkBufferMemoryBarrier2>>(bufferBarriers) },
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
	
	
    PipelineBarrier::PipelineBarrier(PipelineBarrier& copySource) :
        memoryBarriers{ std::move(copySource.memoryBarriers) },
        imageBarriers{ std::move(copySource.imageBarriers) },
        bufferBarriers{ std::move(copySource.bufferBarriers) },
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
    {
		FixPointers();
	}
    PipelineBarrier& PipelineBarrier::operator=(PipelineBarrier& copySource) {
        memoryBarriers = std::move(copySource.memoryBarriers);
        imageBarriers = std::move(copySource.imageBarriers);
        bufferBarriers = std::move(copySource.bufferBarriers);
		
		FixPointers();

        return *this;
    }
    PipelineBarrier::PipelineBarrier(PipelineBarrier&& moveSource) noexcept :
        memoryBarriers{ std::move(moveSource.memoryBarriers) },
        imageBarriers{ std::move(moveSource.imageBarriers) },
        bufferBarriers{ std::move(moveSource.bufferBarriers) },
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
    {
		FixPointers();
	}
    PipelineBarrier& PipelineBarrier::operator=(PipelineBarrier&& moveSource) noexcept {
        memoryBarriers = std::move(moveSource.memoryBarriers);
        imageBarriers = std::move(moveSource.imageBarriers);
        bufferBarriers = std::move(moveSource.bufferBarriers);
		
		FixPointers();

        return *this;
    }

	//the parameter object passed in is no longer usable, submitting both barriers will potentially lead to errors
	void PipelineBarrier::Merge(PipelineBarrier const& other) {
		//idk if i need if operators for empty vectors
		std::copy(other.memoryBarriers.begin(), other.memoryBarriers.end(), std::back_inserter(memoryBarriers));
		std::copy(other.bufferBarriers.begin(), other.bufferBarriers.end(), std::back_inserter(bufferBarriers));
		std::copy(other.imageBarriers.begin(), other.imageBarriers.end(), std::back_inserter(imageBarriers));
	}

	/* i believe this doesnt work with VkDependencyInfo Barrier2
	void PipelineBarrier::SimplifyVector(std::vector<PipelineBarrier>& barriers) {
		if (barriers.size() <= 1) {
			return;
		}
#if EWE_DEBUG
		assert(barriers.size() < 256 && "too many barriers"); //reduce the barrier count. if not possible, change the values and data types here
#endif

        uint8_t lastComparisonIndex = 0;
		uint8_t currentComparisonIndex = 1;
		uint8_t nextComparisonIndex = -1;

		//c short for comparison
        while (currentComparisonIndex < barriers.size()) {
            nextComparisonIndex = static_cast<uint8_t>(barriers.size());

            const VkPipelineStageFlagBits cSrcStageMask = barriers[currentComparisonIndex].srcStageMask;
            const VkPipelineStageFlagBits cDstStageMask = barriers[currentComparisonIndex].dstStageMask;
            const VkDependencyFlags cDependencyFlags = barriers[currentComparisonIndex].dependencyFlags;

            for ( ; currentComparisonIndex < static_cast<uint8_t>(barriers.size()); currentComparisonIndex++) {
                const bool srcComp{ cSrcStageMask == barriers[currentComparisonIndex].srcStageMask };
                const bool dstComp{ cDstStageMask == barriers[currentComparisonIndex].dstStageMask };
                const bool dfComp{ cDependencyFlags == barriers[currentComparisonIndex].dependencyFlags };
                if (srcComp && dstComp && dfComp) {
                    barriers[lastComparisonIndex].Merge(barriers[currentComparisonIndex]);
                    barriers.erase(barriers.begin() + currentComparisonIndex);
                    currentComparisonIndex--;
                }
                else if (nextComparisonIndex > currentComparisonIndex) {
                    nextComparisonIndex = currentComparisonIndex;
                }
            }
            lastComparisonIndex = nextComparisonIndex;
            currentComparisonIndex = lastComparisonIndex + 1;
        }
	}
	*/	
	
	void PipelineBarrier::FixPointers() {
		dependencyInfo.memoryBarrierCount = static_cast<uint32_t>(this->memoryBarriers.size());
		dependencyInfo.pMemoryBarriers = this->memoryBarriers.data();
		dependencyInfo.bufferMemoryBarrierCount = static_cast<uint32_t>(this->bufferBarriers.size());
		dependencyInfo.pBufferMemoryBarriers = this->bufferBarriers.data();
		dependencyInfo.imageMemoryBarrierCount = static_cast<uint32_t>(this->imageBarriers.size());
		dependencyInfo.pImageMemoryBarriers = this->imageBarriers.data();
	}

	namespace Barrier {
		
		VkImageMemoryBarrier2 Acquire_Image(Queue& dstQueue, Resource<Image>& resource, uint8_t frameIndex){
			auto& img = *resource.resource[frameIndex];
			//making them both equal is the same as VK_QUEUE_FAMILY_IGNORED
			const uint32_t srcQueueFamilyIndex = resource.resource[frameIndex]->owningQueue ? resource.resource[frameIndex]->owningQueue->FamilyIndex() : dstQueue.FamilyIndex(); 
			return VkImageMemoryBarrier2{
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
				.pNext = nullptr,
				.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
				.srcAccessMask = VK_ACCESS_2_NONE,
				.dstStageMask = resource.usage.stage,
				.dstAccessMask = resource.usage.accessMask, 
				.oldLayout = img.layout,
				.newLayout = resource.usage.layout,
				.srcQueueFamilyIndex = srcQueueFamilyIndex,
				.dstQueueFamilyIndex = dstQueue.FamilyIndex(),
				.image = img.image,
				.subresourceRange = VkImageSubresourceRange{
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.baseMipLevel = 0,
					.levelCount = img.mipLevels,
					.baseArrayLayer = 0,
					.layerCount = img.arrayLayers
				}
			};
		}
		VkBufferMemoryBarrier2 Acquire_Buffer(Queue& dstQueue, Resource<Buffer>& resource, uint8_t frameIndex){
			const uint32_t srcQueueFamilyIndex = resource.resource[frameIndex]->owningQueue ? resource.resource[frameIndex]->owningQueue->FamilyIndex() : dstQueue.FamilyIndex();
			return VkBufferMemoryBarrier2{
				.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
				.pNext = nullptr,
				.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
				.srcAccessMask = VK_ACCESS_2_NONE,
				.dstStageMask = resource.usage.stage,
				.dstAccessMask = resource.usage.accessMask,
				.srcQueueFamilyIndex = srcQueueFamilyIndex, //if this is nullptr, this field needs to be equal to dstQueueFamilyIndex
				.dstQueueFamilyIndex = dstQueue.FamilyIndex(),
				.buffer = resource.resource[frameIndex]->buffer_info.buffer,
				.offset = 0,
				.size = resource.resource[frameIndex]->bufferSize
			};
		}
		
		VkImageMemoryBarrier2 Transition_Image(Queue& srcQueue, Resource<Image>& lh_resource, Queue& dstQueue, Resource<Image>& rh_resource, uint8_t frameIndex) {
			auto& lh_img = *lh_resource.resource[frameIndex];
			auto& rh_img = *rh_resource.resource[frameIndex];
#if EWE_DEBUG_BOOL
			if (lh_resource.resource[frameIndex]->image != rh_resource.resource[frameIndex]->image) {
				printf("transitioning invalid? - {%s}:{%s}\n", lh_img.name.c_str(), rh_img.name.c_str());
			}
			assert(lh_resource.resource[frameIndex]->image == rh_resource.resource[frameIndex]->image);
#endif
			return VkImageMemoryBarrier2{
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
				.pNext = nullptr,
				.srcStageMask = lh_resource.usage.stage,
				.srcAccessMask = lh_resource.usage.accessMask,
				.dstStageMask = rh_resource.usage.stage,
				.dstAccessMask = rh_resource.usage.accessMask,
				.oldLayout = lh_resource.usage.layout, 
				.newLayout = rh_resource.usage.layout,
				.srcQueueFamilyIndex = srcQueue.FamilyIndex(),
				.dstQueueFamilyIndex = dstQueue.FamilyIndex(),
				.image = rh_img.image,
				.subresourceRange = VkImageSubresourceRange{
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.baseMipLevel = 0,
					.levelCount = rh_img.mipLevels,
					.baseArrayLayer = 0,
					.layerCount = rh_img.arrayLayers
				}
			};
		}
		VkBufferMemoryBarrier2 Transition_Buffer(Queue& srcQueue, Resource<Buffer>& lh_resource, Queue& dstQueue, Resource<Buffer>& rh_resource, uint8_t frameIndex){
			return VkBufferMemoryBarrier2{
				.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
				.pNext = nullptr,
				.srcStageMask = lh_resource.usage.stage,
				.srcAccessMask = lh_resource.usage.accessMask,
				.dstStageMask = rh_resource.usage.stage,
				.dstAccessMask = rh_resource.usage.accessMask,
				.srcQueueFamilyIndex = srcQueue.FamilyIndex(),
				.dstQueueFamilyIndex = dstQueue.FamilyIndex(),
				.buffer = rh_resource.resource[frameIndex]->buffer_info.buffer,
				.offset = 0,
				.size = rh_resource.resource[frameIndex]->bufferSize
			};
		}
        
	}//namespace Barrier

} //namespace EWE