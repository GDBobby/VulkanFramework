#include "EightWinds/Backend/RenderInfo3.h"

#include "EightWinds/Backend/STC_Helper.h"

#include <memory>

namespace EWE{
	
	void SetImageData(Image& image, Queue& queue, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VmaAllocationCreateInfo& vmaAllocCreateInfo){
		image.arrayLayers = 1;
		image.extent = { width, height, 1 };
		image.mipLevels = 1;
		image.owningQueue = &queue;
		image.samples = VK_SAMPLE_COUNT_1_BIT;
		image.tiling = VK_IMAGE_TILING_OPTIMAL;
		image.type = VK_IMAGE_TYPE_2D;
		image.format = format;
		image.usage = usage;
		image.Create(vmaAllocCreateInfo);
	}
	
	RenderInfo3::RenderInfo3(
		std::string_view name,
		LogicalDevice& logicalDevice,
		Queue& graphicsQueue,
		uint32_t width, uint32_t height,
		std::vector<AttachmentConstructionInfo> const& color_infos,
		AttachmentConstructionInfo depth_info
	)
	: name{name},
		logicalDevice{logicalDevice},
		graphicsQueue{graphicsQueue},
		//this should pass args to PerFlight
		//which puts the args to each (per frame in flight) RuntimeArray
		//which will construct a color_formats.size() amount of images with the argument, logicalDevice
		color_images{ color_infos.size(), logicalDevice},
		color_views{ color_images},
		depth_images{logicalDevice},
		depth_views{depth_images},
		depth_format{depth_info.format}
	{
		color_formats.resize(color_infos.size());
		for (uint8_t i = 0; i < color_formats.size(); i++) {
			color_formats[i] = color_infos[i].format;
		}

		CreateImages(name, width, height, color_infos);

		InitialTransition();
	}
	
	void RenderInfo3::InitialTransition() {
		const std::size_t color_image_count = color_images.Size() * EWE::max_frames_in_flight;
		const std::size_t depth_image_count = EWE::max_frames_in_flight;
		const std::size_t total_image_count = color_image_count + depth_image_count;

		RuntimeArray<VkImageMemoryBarrier2> transition_barriers{total_image_count};

		for (uint8_t i = 0; i < color_image_count; i++) {
			const std::size_t frame = i >= color_images.Size();
			const std::size_t image_index = i % color_images.Size();

			auto& barr = transition_barriers[i];
			barr.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
			barr.pNext = nullptr;
			barr.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barr.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barr.srcAccessMask = VK_ACCESS_2_NONE;
			barr.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
			barr.image = color_images[image_index][frame].image;
			barr.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			barr.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			barr.subresourceRange = EWE::ImageView::GetDefaultSubresource(color_images[image_index][frame]);
			barr.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
			barr.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
		}

		for (std::size_t i = 0; i < depth_image_count; i++) {
			auto& barr = transition_barriers[i + color_image_count];
			barr.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
			barr.pNext = nullptr;
			barr.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barr.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barr.srcAccessMask = VK_ACCESS_2_NONE;
			barr.dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			barr.image = depth_images[i].image;
			barr.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			barr.newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
			barr.subresourceRange = EWE::ImageView::GetDefaultSubresource(depth_images[i]);
			barr.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
			barr.dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT;
		}

		VkDependencyInfo transition_dependency{
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			.pNext = nullptr,
			.dependencyFlags = 0,
			.memoryBarrierCount = 0,
			.bufferMemoryBarrierCount = 0,
			.imageMemoryBarrierCount = static_cast<uint32_t>(transition_barriers.Size()),
			.pImageMemoryBarriers = transition_barriers.heap.GetMemory()
		};

		Command_Helper::Transition(logicalDevice, graphicsQueue, transition_dependency, true);

		for (uint8_t i = 0; i < color_image_count; i++) {
			const std::size_t frame = i >= color_images.Size();
			const std::size_t image_index = i % color_images.Size();
			color_images[image_index][frame].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
	}
	
	
	void RenderInfo3::CreateImages(std::string_view name, uint32_t width, uint32_t height, std::vector<AttachmentConstructionInfo> const& color_infos){
		VmaAllocationCreateInfo vmaAllocCreateInfo{
		//if(imageCreateInfo.width * height > some amount){
			.flags = static_cast<VmaAllocationCreateFlags>(VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT) |
										static_cast<VmaAllocationCreateFlags>(VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT),
			.usage = VMA_MEMORY_USAGE_AUTO
		};
		
		
		for (uint8_t frame = 0; frame < EWE::max_frames_in_flight; frame++) {
			for(uint8_t i = 0; i < color_images.Size(); i++){
				SetImageData(color_images[frame][i], graphicsQueue, width, height, color_infos[i].format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, vmaAllocCreateInfo);
#if EWE_DEBUG_NAMING
				const std::string resource_name = "color[" + std::to_string(frame) + "][" + std::to_string(i) + "]";
				color_images[i][frame].SetName(resource_name);
				
#endif
			}
			SetImageData(depth_images[frame], graphicsQueue, width, height, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, vmaAllocCreateInfo);
#if EWE_DEBUG_NAMING
			std::string resource_name = "depth[" + std::to_string(frame) + "]";
				depth_images[frame].SetName(resource_name);
#endif
		}
	}
}