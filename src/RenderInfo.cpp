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
		for (uint8_t i = 0; i < attachments.color_views.Size(); i++) {
			auto& caiv = attachments.color_views[i][frameIndex];
			colors.push_back(
				VkRenderingAttachmentInfo{
					.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
					.pNext = nullptr,
					.imageView = caiv->view,
					.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
					.resolveMode = VK_RESOLVE_MODE_NONE,
					.resolveImageView = VK_NULL_HANDLE,
					.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
					.loadOp = attachments.setInfo.colors[i].loadOp,
					.storeOp = attachments.setInfo.colors[i].storeOp,
					.clearValue = attachments.setInfo.colors[i].clearValue
				}
			);
		}
		if(attachments.setInfo.using_depth){
			EWE_ASSERT(attachments.depth_views[frameIndex] != nullptr);
			depth = VkRenderingAttachmentInfo{
				.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
				.pNext = nullptr,
				.imageView = attachments.depth_views[frameIndex]->view,
				.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
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

	Render_Vk_Data::Render_Vk_Data(RenderAttachments const& attachments, uint32_t screen_width, uint32_t screen_height)
		: vk_data{ ArgumentPack_ConstructionHelper<2>{}, attachments, 0, attachments, 1 } //only going to work if max frames in flight is 2
	{
		for (uint8_t i = 0; i < max_frames_in_flight; i++) {
			vk_info[i] = VkRenderingInfo{
				.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
				.pNext = nullptr,
				.flags = attachments.setInfo.renderingFlags,
				.renderArea = attachments.setInfo.CalculateRenderArea(screen_width, screen_height),
				.layerCount = 1,
				.viewMask = 0,
				.colorAttachmentCount = static_cast<uint32_t>(vk_data[i].colors.size()),
				.pColorAttachments = vk_data[i].colors.data(),
				.pDepthAttachment = attachments.setInfo.using_depth ? &vk_data[i].depth : nullptr,
				.pStencilAttachment = nullptr
			};
		}
	}



	VkRect2D AttachmentSetInfo::CalculateRenderArea(uint32_t screen_width, uint32_t screen_height) const{
		//if we're not enforcing uniform size, render area will be equal to the smallest size here
		if(relative_size){
			return VkRect2D{
				.offset{
					.x = 0,
					.y = 0
				},
				.extent{
					.width = static_cast<uint32_t>(width * static_cast<float>(screen_width)),
					.height = static_cast<uint32_t>(height * static_cast<float>(screen_height))
				}
			};
		}
		else{
			return VkRect2D{
				.offset{
					.x = 0,
					.y = 0
				},
				.extent{
					.width = width,
					.height = height
				}
			};
		}
	}

	void FullRenderInfo::Undefer(InstructionPointer<ParamPack<Inst::BeginRender>>* deferred_render_info){
		for_each_frame{
			//this reference is just for debugging purposes
			auto& temp_info = render_data.vk_info[frame];
			deferred_render_info->GetRef(frame) = temp_info;
		}
	}

	void SetImageData(Image& image, Queue& queue, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VmaAllocationCreateInfo const& vmaAllocCreateInfo) {
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

	void GenerateImage(Image* img_ptr, ImageView* view_ptr, uint32_t width, uint32_t height){
		
	}

	void RenderAttachments::GenerateImage(
		PerFlight<Image*> img_con_addr, 
		PerFlight<ImageView*> view_con_addr, 
		uint32_t width, uint32_t height,
		int index
	) {
		const VmaAllocationCreateInfo vmaAllocCreateInfo{
		//if(imageCreateInfo.width * height > some amount){
			.flags = static_cast<VmaAllocationCreateFlags>(VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT) |
					static_cast<VmaAllocationCreateFlags>(VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT),
			.usage = VMA_MEMORY_USAGE_AUTO
		};

		for_each_frame{
			if(img_con_addr[frame] == nullptr){
				img_con_addr[frame] = new Image(logicalDevice);
			}
			else{
				std::construct_at(img_con_addr[frame], logicalDevice);
			}

			if(index < 0){ //if index is less than 0, its the depth image

				//generating depth
				SetImageData(*img_con_addr[frame], graphicsQueue, 
					width, height, 
					setInfo.depth.format, 
					VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
					vmaAllocCreateInfo
				);

				if(view_con_addr[frame] != nullptr){
					std::construct_at(view_con_addr[frame], *img_con_addr[frame]);
					depth_views[frame] = view_con_addr[frame];
				}
				else{
					depth_views[frame] = new ImageView(*img_con_addr[frame]);
				}
#if EWE_DEBUG_NAMING
				const std::string resource_name = "generated["
												+ std::to_string(reinterpret_cast<std::size_t>(this))
												+ "] depth[" + std::to_string(frame) + "]";
				depth_views[frame]->image.SetName(resource_name);
#endif
			}
			else{ //index is into the color array

				SetImageData(*img_con_addr[frame], graphicsQueue, 
					width, height, 
					setInfo.colors[index].format, 
					VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
					vmaAllocCreateInfo
				);
				if(view_con_addr[frame] != nullptr){
					std::construct_at(view_con_addr[frame], *img_con_addr[frame]);
					color_views[index][frame] = view_con_addr[frame];
				}
				else{
					color_views[index][frame] = new ImageView(*img_con_addr[frame]);
				}
#if EWE_DEBUG_NAMING
				const std::string resource_name = "generated[" + 
												std::to_string(reinterpret_cast<std::size_t>(this))
												+ "] color[" + std::to_string(index) 
												+ "][" + std::to_string(frame) + "]";
				color_views[index][frame]->image.SetName(resource_name);

#endif
			}
			img_con_addr[frame]->data.layout = VK_IMAGE_LAYOUT_UNDEFINED;
		}
	}
	void RenderAttachments::CreateImages(uint32_t width, uint32_t height){
		for(std::size_t i = 0; i < color_views.Size(); i++){
			if(color_views[i][0] == nullptr){
				//potentially assert frame[1] is also nullptr
				GenerateImage(PerFlight<Image*>{nullptr}, color_views[i], width, height, i);
			}
			if(setInfo.using_depth){
				GenerateImage(PerFlight<Image*>{nullptr}, depth_views[i], width, height, -1);
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
		using_depth{true},
		depth{_depth}
	{
		for(std::size_t i = 0; i < _colors.size(); i++){
			colors[i] = _colors[i];
		}
	}
	AttachmentSetInfo::AttachmentSetInfo(
		uint32_t _width, uint32_t _height, 
		VkRenderingFlags _renderingFlags,
		std::span<const AttachmentInfo> _colors
	)	
	: width{_width}, height{_height},
		renderingFlags{_renderingFlags},
		colors{_colors.size()},
		using_depth{false},
		depth{}
	{
		for(std::size_t i = 0; i < _colors.size(); i++){
			colors[i] = _colors[i];
		}
	}

	AttachmentSetInfo::AttachmentSetInfo(AttachmentSetInfo const& copySrc)
	: relative_size{copySrc.relative_size},
		width{copySrc.width},
		height{copySrc.height},
		renderingFlags{copySrc.renderingFlags},
		colors{copySrc.colors.Size()},
		using_depth{copySrc.using_depth},
		depth{copySrc.depth}
	{
		for(std::size_t i = 0; i < colors.Size(); i++){
			colors[i] = copySrc.colors[i];
		}
	}

	AttachmentSetInfo& AttachmentSetInfo::operator=(AttachmentSetInfo const& copySrc){
		relative_size = copySrc.relative_size;
		width = copySrc.width;
		height = copySrc.height;
		renderingFlags = copySrc.renderingFlags;
		colors.ClearAndResize(copySrc.colors.Size());

		using_depth = copySrc.using_depth;
		depth = copySrc.depth;
		
		for(std::size_t i = 0; i < colors.Size(); i++){
			colors[i] = copySrc.colors[i];
		}
		return *this;
	}

	RenderAttachments::RenderAttachments(
		std::filesystem::path const& _name,
		LogicalDevice& _logicalDevice,
		Queue& _graphicsQueue,
		AttachmentSetInfo const& _setInfo,
		uint32_t width, uint32_t height
	)
		: name{ _name },
		logicalDevice{ _logicalDevice },
		graphicsQueue{ _graphicsQueue },
		setInfo{_setInfo},
		meta{setInfo.colors.Size() + setInfo.using_depth},
		color_views{ _setInfo.colors.Size(), nullptr },
		depth_views{ nullptr }
	{
		for(auto& src : meta){
			src.src_owner = reinterpret_cast<FullRenderInfo*>(this);
		}
		CreateImages(width, height);
		InitialTransition();
	}
	RenderAttachments::RenderAttachments(
		std::filesystem::path const& _name,
		LogicalDevice& _logicalDevice,
		Queue& _graphicsQueue,
		AttachmentSetInfo const& _setInfo,
		std::span<const AttachmentMeta> _meta,
		uint32_t width, uint32_t height
	)
	: name{ _name },
		logicalDevice{ _logicalDevice },
		graphicsQueue{ _graphicsQueue },
		setInfo{_setInfo},
		meta{_meta.size()},
		color_views{ _setInfo.colors.Size(), nullptr },
		depth_views{ nullptr }
	{
		for(std::size_t i = 0; i < meta.Size(); i++){
			meta[i].src_owner = _meta[i].src_owner;
			meta[i].src_index = _meta[i].src_index;
		}
		for(std::size_t i = 0; i < _setInfo.colors.Size(); i++){
			for(auto& info : meta){
				if(info.src_owner == reinterpret_cast<FullRenderInfo*>(this)){
					//generate
					//CreateImages already accounts for partial construction
				}
				else if(info.src_owner != nullptr){
					for_each_frame{
						if(info.src_index >= 0){
							color_views[i][frame] = info.src_owner->full.color_views[info.src_index][frame];
						}
						else{
							depth_views[frame] = info.src_owner->full.depth_views[frame];
						}
					}
				}
			}
		}
		CreateImages(width, height);
		InitialTransition();
	}

	void RenderAttachments::InitialTransition() {

		std::vector<Image*> color_images{};
		for(auto& view : color_views){
			for_each_frame{
				EWE_ASSERT(view[frame] != nullptr);
				if(view[frame]->image.data.layout == VK_IMAGE_LAYOUT_UNDEFINED){
					color_images.push_back(&view[frame]->image);
				}
			}
		}
		const std::size_t color_image_count = color_images.size();
		std::size_t total_image_count = color_image_count;

		PerFlight<bool> trans_depth{false};
		for_each_frame{
			if(depth_views[frame] != nullptr){
				if(depth_views[frame]->image.data.layout == VK_IMAGE_LAYOUT_UNDEFINED){
					total_image_count++;
					trans_depth[frame] = true;
				}
			}
		}

		RuntimeArray<VkImageMemoryBarrier2> transition_barriers{ total_image_count };

		std::size_t barrier_index = 0;
		for (auto* img : color_images) {

			auto& barr = transition_barriers[barrier_index];
			barr.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
			barr.pNext = nullptr;
			barr.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barr.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barr.srcAccessMask = VK_ACCESS_2_NONE;
			barr.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
			barr.image = img->image;
			barr.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			barr.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			barr.subresourceRange = EWE::ImageView::GetDefaultSubresource(*img);
			barr.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
			barr.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

			barrier_index++;
		}

		for_each_frame{
			if(trans_depth[frame]){
				auto& barr = transition_barriers[barrier_index];
				barr.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
				barr.pNext = nullptr;
				barr.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barr.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barr.srcAccessMask = VK_ACCESS_2_NONE;
				barr.dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				barr.image = depth_views[frame]->image.image;
				barr.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				barr.newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
				barr.subresourceRange = EWE::ImageView::GetDefaultSubresource(depth_views[frame]->image);
				barr.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
				barr.dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT;

				barrier_index++;
			}
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

		for (auto& view : color_views) {
			for_each_frame{
				auto& img = view[frame]->image;
				if(img.data.layout == VK_IMAGE_LAYOUT_UNDEFINED){
					img.data.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				}
				else{
					EWE_ASSERT(img.data.layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 
						"the image alreayd had a layout, and it's invalid");
				}
			}
		}
	}

	FullRenderInfo::FullRenderInfo(
		std::filesystem::path const& _name,
		LogicalDevice& _logicalDevice,
		Queue& _graphicsQueue,
		AttachmentSetInfo const& _setInfo,
		uint32_t width, uint32_t height
	)
	: full{_name, _logicalDevice, _graphicsQueue, _setInfo, width, height},
		render_data{full, width, height},
		name{full.name}
	{
	}
	FullRenderInfo::FullRenderInfo(
		std::filesystem::path const& _name,
		LogicalDevice& _logicalDevice,
		Queue& _graphicsQueue,
		AttachmentSetInfo const& _setInfo,
		std::span<const AttachmentMeta> _meta,
		uint32_t width, uint32_t height
	)
	: full{_name, _logicalDevice, _graphicsQueue, _setInfo, _meta, width, height},
		render_data{full, width, height},
		name{full.name}
	{
	}
}// namespace EWE