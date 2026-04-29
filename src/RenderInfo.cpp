#include "EightWinds/Backend/RenderInfo.h"

#include "EightWinds/ImageView.h"

#include "EightWinds/Backend/STC_Helper.h"
#include <cstddef>
#include <vulkan/vulkan_core.h>

/*
VUID-VkRenderingAttachmentInfo-pNext-11752
If the pNext chain includes a VkRenderingAttachmentFlagsInfoKHR structure, and flags includes VK_RENDERING_ATTACHMENT_RESOLVE_SKIP_TRANSFER_FUNCTION_BIT_KHR or VK_RENDERING_ATTACHMENT_RESOLVE_ENABLE_TRANSFER_FUNCTION_BIT_KHR, imageView must have a format using sRGB encoding

VUID-VkRenderingAttachmentInfo-pNext-11753
If the pNext chain includes a VkRenderingAttachmentFlagsInfoKHR structure, and flags includes VK_RENDERING_ATTACHMENT_RESOLVE_SKIP_TRANSFER_FUNCTION_BIT_KHR or VK_RENDERING_ATTACHMENT_RESOLVE_ENABLE_TRANSFER_FUNCTION_BIT_KHR, resolveMode must be equal to VK_RESOLVE_MODE_AVERAGE_BIT

VUID-VkRenderingAttachmentInfo-pNext-11754
If the pNext chain includes a VkRenderingAttachmentFlagsInfoKHR structure, and flags includes VK_RENDERING_ATTACHMENT_INPUT_ATTACHMENT_FEEDBACK_BIT_KHR, imageView must have an image that was created with the VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT usage flag set

Each pNext member of any structure (including this one) in the pNext chain must be either NULL or a pointer to a valid instance of VkAttachmentFeedbackLoopInfoEXT or VkRenderingAttachmentFlagsInfoKHR

*/

namespace EWE{
	void RenderInfo::Init(RenderAttachments const& attachments, uint8_t frameIndex){
		for (uint8_t i = 0; i < attachments.color_images.Size(); i++) {
			auto& caiv = attachments.color_views[i][frameIndex];
			colors.push_back(
				VkRenderingAttachmentInfo{
					.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
					.pNext = nullptr,
					.imageView = caiv->view,
					.imageLayout = caiv->image.data.layout,
					.resolveMode = VK_RESOLVE_MODE_NONE,
					.resolveImageView = VK_NULL_HANDLE,
					.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
					.loadOp = attachments.setInfo.colors[i].loadOp,
					.storeOp = attachments.setInfo.colors[i].storeOp,
					.clearValue = attachments.setInfo.colors[i].clearValue
				}
			);
		}
		if(attachments.depth_views[frameIndex] != nullptr){
			depth = VkRenderingAttachmentInfo{
				.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
				.pNext = nullptr,
				.imageView = attachments.depth_views[frameIndex]->view,
				.imageLayout = attachments.depth_views[frameIndex]->image.data.layout,
				.resolveMode = VK_RESOLVE_MODE_NONE,
				.resolveImageView = VK_NULL_HANDLE,
				.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.loadOp = attachments.setInfo.depth.loadOp,
				.storeOp = attachments.setInfo.depth.storeOp,
				.clearValue = attachments.setInfo.depth.clearValue
			};
		}
		else{
			depth.sType = VK_STRUCTURE_TYPE_MAX_ENUM;
		}
	}

	RenderInfo::RenderInfo(RenderAttachments const& attachments, uint8_t frameIndex) {
		Init(attachments, frameIndex);
	}


	void Render_Vk_Data::Init(RenderAttachments const& attachments, VkRenderingFlags renderingFlags)
	{
		for(uint8_t frame = 0; frame < max_frames_in_flight; frame++){
			vk_data[frame].Init(attachments, frame);
		}
		for (uint8_t i = 0; i < max_frames_in_flight; i++) {
			vk_info[i] = VkRenderingInfo{
				.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
				.pNext = nullptr,
				.flags = renderingFlags,
				.renderArea = attachments.setInfo.CalculateRenderArea(),
				.layerCount = 1,
				.viewMask = 0,
				.colorAttachmentCount = static_cast<uint32_t>(vk_data[i].colors.size()),
				.pColorAttachments = vk_data[i].colors.data(),
				.pDepthAttachment = vk_data[i].depth.sType != VK_STRUCTURE_TYPE_MAX_ENUM ? &vk_data[i].depth : nullptr,
				.pStencilAttachment = nullptr
			};
		}
	}

	Render_Vk_Data::Render_Vk_Data(RenderAttachments const& attachments, VkRenderingFlags renderingFlags)
		: vk_data{ ArgumentPack_ConstructionHelper<2>{}, attachments, 0, attachments, 1 } //only going to work if max frames in flight is 2
	{
		for (uint8_t i = 0; i < max_frames_in_flight; i++) {
			vk_info[i] = VkRenderingInfo{
				.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
				.pNext = nullptr,
				.flags = renderingFlags,
				.renderArea = attachments.setInfo.CalculateRenderArea(),
				.layerCount = 1,
				.viewMask = 0,
				.colorAttachmentCount = static_cast<uint32_t>(vk_data[i].colors.size()),
				.pColorAttachments = vk_data[i].colors.data(),
				.pDepthAttachment = &vk_data[i].depth,
				.pStencilAttachment = nullptr
			};
		}
	}



	VkRect2D AttachmentSetInfo::CalculateRenderArea() const{
		VkRect2D ret{};
		ret.offset.x = 0;
		ret.offset.y = 0;
		//if we're not enforcing uniform size, render area will be equal to the smallest size here
		ret.extent.width = width;
		ret.extent.height = height;
		return ret;
	}

	void FullRenderInfo::Undefer(InstructionPointer<VkRenderingInfo>* deferred_render_info){
		for(uint8_t frame = 0; frame < max_frames_in_flight; frame++){
			auto& temp_info = render_data.vk_info[frame];
			deferred_render_info->GetRef(frame) = temp_info;
		}
	}

	void SetImageData(Image& image, Queue& queue, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VmaAllocationCreateInfo& vmaAllocCreateInfo) {
		image.data.arrayLayers = 1;
		image.data.extent = { width, height, 1 };
		image.data.mipLevels = 1;
		image.owningQueue = &queue;
		image.data.samples = VK_SAMPLE_COUNT_1_BIT;
		image.data.tiling = VK_IMAGE_TILING_OPTIMAL;
		image.data.type = VK_IMAGE_TYPE_2D;
		image.data.format = format;
		image.data.usage = usage;
		image.Create(vmaAllocCreateInfo);
	}


	void RenderAttachments::CreateImages(uint32_t width, uint32_t height) {
		VmaAllocationCreateInfo vmaAllocCreateInfo{
		//if(imageCreateInfo.width * height > some amount){
			.flags = static_cast<VmaAllocationCreateFlags>(VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT) |
					static_cast<VmaAllocationCreateFlags>(VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT),
			.usage = VMA_MEMORY_USAGE_AUTO
		};


		for_each_frame {
			for (uint8_t i = 0; i < color_images.Size(); i++) {
				SetImageData(*color_images[i][frame], graphicsQueue, width, height, setInfo.colors[i].format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, vmaAllocCreateInfo);
				color_images[i][frame]->data.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
#if EWE_DEBUG_NAMING
				const std::string resource_name = "color[" + std::to_string(frame) + "][" + std::to_string(i) + "]";
				color_images[i][frame]->SetName(resource_name);

#endif
			}
			if(depth_image[frame] != nullptr){
				SetImageData(*depth_image[frame], graphicsQueue, width, height, setInfo.depth.format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, vmaAllocCreateInfo);
				depth_image[frame]->data.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
#if EWE_DEBUG_NAMING
				const std::string resource_name = "depth[" + std::to_string(frame) + "]";
				depth_image[frame]->SetName(resource_name);
#endif
			}
		}
	}
	AttachmentSetInfo::AttachmentSetInfo(
		uint32_t _width, uint32_t _height, 
		VkRenderingFlags _renderingFlags,
		std::span<const AttachmentInfo> _colors,
		AttachmentInfo _depth 
	)	
	: width{_width}, height{_height},
		renderingFlags{_renderingFlags},
		colors{_colors.size()},
		depth{_depth}
	{
		for(std::size_t i = 0; i < _colors.size(); i++){
			colors[i] = _colors[i];
		}
	}

	AttachmentSetInfo::AttachmentSetInfo(AttachmentSetInfo const& copySrc)
	: width{copySrc.width},
		height{copySrc.height},
		renderingFlags{copySrc.renderingFlags},
		colors{copySrc.colors.Size()},
		depth{copySrc.depth}
	{
		for(std::size_t i = 0; i < colors.Size(); i++){
			colors[i] = copySrc.colors[i];
		}
	}

	void RenderAttachments::Init(){

		CreateImages(setInfo.width, setInfo.height);

		InitialTransition();
		CreateImageViews();
	}

	RenderAttachments::RenderAttachments(
		std::string_view _name,
		LogicalDevice& _logicalDevice,
		Queue& _graphicsQueue,
		AttachmentSetInfo const& _setInfo
	)
		: name{ _name },
		logicalDevice{ _logicalDevice },
		graphicsQueue{ _graphicsQueue },
		//this should pass args to PerFlight
		//which puts the args to each (per frame in flight) RuntimeArray
		//which will construct a color_formats.size() amount of images with the argument, logicalDevice
		color_images{ _setInfo.colors.Size(), nullptr },
		color_views{ _setInfo.colors.Size() },
		depth_image{ nullptr },
		depth_views{ nullptr },
		setInfo{_setInfo}
	{
	}

	void RenderAttachments::InitialTransition() {
		const std::size_t color_image_count = color_images.Size() * EWE::max_frames_in_flight;
		const std::size_t depth_image_count = EWE::max_frames_in_flight * static_cast<uint8_t>(depth_image[0] != nullptr);
		const std::size_t total_image_count = color_image_count + depth_image_count;

		RuntimeArray<VkImageMemoryBarrier2> transition_barriers{ total_image_count };

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
			barr.image = color_images[image_index][frame]->image;
			barr.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			barr.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			barr.subresourceRange = EWE::ImageView::GetDefaultSubresource(*color_images[image_index][frame]);
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
			barr.image = depth_image[i]->image;
			barr.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			barr.newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
			barr.subresourceRange = EWE::ImageView::GetDefaultSubresource(*depth_image[i]);
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
			color_images[image_index][frame]->data.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
	}

	void RenderAttachments::CreateImageViews() {
		for (uint8_t i = 0; i < color_images.Size(); i++) {
			for(uint8_t frame = 0; frame < max_frames_in_flight; frame++){
				color_views[i][frame] = new ImageView(*color_images[i][frame]);
				//printf("view : img - i : frame - %zu : %zu - %u : %u\n", 
				//	reinterpret_cast<std::size_t>(color_views[i][frame]->image.image), 
				//	reinterpret_cast<std::size_t>(color_images[i][frame]->image), 
				//	i, frame
				//);
			}
		}
		if(depth_image[0] == nullptr){
			for(uint8_t frame = 0; frame < max_frames_in_flight; frame++){
				depth_views[frame] = nullptr;
			}
		}
		else{
			for(uint8_t frame = 0; frame < max_frames_in_flight; frame++){
				depth_views[frame] = new ImageView(*depth_image[frame]);
			}
		}
	}

	void FullRenderInfo::Init(){
		full.color_images.ClearAndResize(full.setInfo.colors.Size());
		full.color_views.ClearAndResize(full.setInfo.colors.Size());

		for(uint8_t i = 0; i < full.setInfo.colors.Size(); i++){
			for(uint8_t frame = 0; frame < EWE::max_frames_in_flight; frame++){
				full.color_images[0][frame] = new EWE::Image(full.logicalDevice);
			}
		}
		full.Init();
		render_data.Init(full, full.setInfo.renderingFlags);
	}

	FullRenderInfo::FullRenderInfo(
		std::string_view _name,
		LogicalDevice& _logicalDevice,
		Queue& _graphicsQueue,
		AttachmentSetInfo const& _setInfo
	)
	: full{_name, _logicalDevice, _graphicsQueue, _setInfo},
			render_data{}
	{

	}
}// namespace EWE