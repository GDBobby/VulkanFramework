#include "EightWinds/RenderGraph/TaskBridge.h"

#include "EightWinds/ImageView.h"
#include "EightWinds/Command/CommandBuffer.h"

namespace EWE {

	template<typename T>
	inline void AddUniqueResource(std::vector<T*>& v, T* ptr) {
		if (ptr == nullptr) {
			return;
		}
		for (auto p : v) {
			if (p == ptr) {
				return;
			}
		}
		v.push_back(ptr);
	}

	struct ResourceCollectionSmall {
		std::vector<Resource<Buffer> const*> buffers;
		std::vector<Resource<Image> const*> images;

		void AddUniqueBuffer(Resource<Buffer> const* buf) {
			if (buf->resource == nullptr) {
				return;
			}
			for (auto const& ptr : buffers) {
				if (ptr == buf) {
					return;
				}
			}
			buffers.push_back(buf);
		}
		void AddUniqueImage(Resource<Image> const* img) {
			if (img->resource == nullptr) {
				return;
			}
			for (auto const& ptr : images) {
				if (ptr == img) {
					return;
				}
			}
			images.push_back(img);
		}

		[[nodiscard]] explicit ResourceCollectionSmall(GPUTask const& task) noexcept {

			for (auto& push : task.pushTrackers) {
				for (uint8_t i = 0; i < GlobalPushConstant::buffer_count; i++) {
					AddUniqueBuffer(&push.buffers[i]);
				}
				for (uint8_t i = 0; i < GlobalPushConstant::texture_count; i++) {
					AddUniqueImage(&push.textures[i]);
				}
			}

			for (auto& blit : task.blitTrackers) {
				AddUniqueImage(&blit.srcImage);
				AddUniqueImage(&blit.dstImage);
			}
		}
	};

	TaskBridge::TaskBridge(GPUTask& lhs, GPUTask& rhs) noexcept
		: logicalDevice{ lhs.logicalDevice },
		lhs{ &lhs },
		rhs{ &rhs },
		name{lhs.name + " -> " + rhs.name}
	{
		dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dependencyInfo.pNext = nullptr;
		dependencyInfo.dependencyFlags = 0; //might need to fine tune this
	}
	TaskBridge::TaskBridge(GPUTask& rhs) noexcept
		: logicalDevice{ rhs.logicalDevice },
		lhs{nullptr},
		rhs{ &rhs },
		name{std::string(":~ " + rhs.name)}
	{
		dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dependencyInfo.pNext = nullptr;
		dependencyInfo.dependencyFlags = 0; //might need to fine tune this
	}

	TaskBridge::TaskBridge(TaskBridge&& moveSrc) noexcept
		: logicalDevice{ lhs->logicalDevice },
		lhs{ moveSrc.lhs },
		rhs{ moveSrc.rhs },
		bufferBarriers{ std::move(moveSrc.bufferBarriers) },
		imageBarriers{ std::move(moveSrc.imageBarriers) },
		bufferBarrierResources{ std::move(moveSrc.bufferBarrierResources) },
		imageBarrierResources{ std::move(moveSrc.imageBarrierResources) }
	{
		dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dependencyInfo.pNext = nullptr;
		dependencyInfo.dependencyFlags = 0; //might need to fine tune this
	}

	void TaskBridge::LeftToRightBarriers(const uint8_t frameIndex, ResourceCollectionSmall& rhsColl) {
		assert(lhs != nullptr);
		ResourceCollectionSmall lhsColl{ *lhs };


		{ //buffers
			VkBufferMemoryBarrier2 buf_bar{};
			buf_bar.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
			buf_bar.pNext = nullptr;
			buf_bar.srcQueueFamilyIndex = lhs->queue.family.index;
			buf_bar.dstQueueFamilyIndex = rhs->queue.family.index;

			for (auto* left : lhsColl.buffers) {
				for (auto* right : rhsColl.buffers) {
					if (left->resource == right->resource) {
						bool leftWrites = GetAccessMaskWrite(left->usage.accessMask);
						bool rightWrites = GetAccessMaskWrite(right->usage.accessMask);

						if (leftWrites || rightWrites) {
							buf_bar.buffer = left->resource->buffer_info.buffer;
							buf_bar.size = left->resource->buffer_info.range;
							buf_bar.offset = left->resource->buffer_info.offset;

							buf_bar.srcStageMask = left->usage.stage;
							buf_bar.srcAccessMask = left->usage.accessMask;

							buf_bar.dstStageMask = right->usage.stage;
							buf_bar.dstAccessMask = right->usage.accessMask;
							bufferBarriers.push_back(buf_bar);
						}
						//resources are unique, so no point in checking the rest if we found a match
						break;
					}
				}
			}
		}

		VkImageMemoryBarrier2 img_bar{};
		img_bar.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		img_bar.pNext = nullptr;
		img_bar.srcQueueFamilyIndex = lhs->queue.family.index;
		img_bar.dstQueueFamilyIndex = rhs->queue.family.index;

		img_bar.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		img_bar.subresourceRange.baseArrayLayer = 0;
		img_bar.subresourceRange.baseMipLevel = 0;
		{ //images

			img_bar.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

			for (auto* left : lhsColl.images) {
				for (auto* right : rhsColl.images) {
					if (left->resource == right->resource) {
						bool leftWrites = GetAccessMaskWrite(left->usage.accessMask);
						bool rightWrites = GetAccessMaskWrite(right->usage.accessMask);

						if (leftWrites || rightWrites) {
							img_bar.image = left->resource->image;

							img_bar.subresourceRange.layerCount = left->resource->arrayLayers;
							img_bar.subresourceRange.levelCount = left->resource->mipLevels;

							img_bar.srcStageMask = left->usage.stage;
							img_bar.srcAccessMask = left->usage.accessMask;
							img_bar.oldLayout = left->layout;

							img_bar.dstStageMask = right->usage.stage;
							img_bar.dstAccessMask = right->usage.accessMask;
							img_bar.newLayout = right->layout;
							imageBarriers.push_back(img_bar);
						}
						//resources are unique, so no point in checking the rest if we found a match
						break;
					}
				}
			}
		}

		{ //attachments	
			std::vector<Resource<Image>> previousAttachments{};

			img_bar.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

			if (lhs->renderTracker != nullptr) {
				std::size_t totalAttachmentCount = lhs->renderTracker->compact.color_attachments.size();
				const bool hasDepth = lhs->renderTracker->compact.depth_attachment.imageView[frameIndex] != nullptr;
				totalAttachmentCount += hasDepth;

				previousAttachments.reserve(totalAttachmentCount);
				for (auto& col_att : lhs->renderTracker->compact.color_attachments) {
					auto& backAtt = previousAttachments.emplace_back();
					backAtt.resource = &col_att.imageView[frameIndex]->image;
					backAtt.usage.stage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
					backAtt.usage.accessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
					backAtt.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				}
				{ //depth
					if (hasDepth) {
						auto& backAtt = previousAttachments.emplace_back();
						backAtt.resource = &lhs->renderTracker->compact.depth_attachment.imageView[frameIndex]->image;
						/*
							VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT specifies the stage of the pipeline where early fragment tests
							(depth and stencil tests before fragment shading) are performed. This stage also includes render pass load
							operations for framebuffer attachments with a depth/stencil format.
						*/
						backAtt.usage.stage = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT;
						backAtt.usage.accessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
						backAtt.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
					}
				}
			}

			for (auto& left : previousAttachments) {
			//now we find matches
				for (auto* right : rhsColl.images) {
					if (left.resource == right->resource) {
						bool leftWrites = GetAccessMaskWrite(left.usage.accessMask);
						bool rightWrites = GetAccessMaskWrite(right->usage.accessMask);

						if (leftWrites || rightWrites) {
							img_bar.image = left.resource->image;

							if (left.layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) {
								img_bar.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
							}
							else {
								img_bar.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
							}

							img_bar.subresourceRange.layerCount = left.resource->arrayLayers;
							img_bar.subresourceRange.levelCount = left.resource->mipLevels;

							img_bar.srcStageMask = left.usage.stage;
							img_bar.srcAccessMask = left.usage.accessMask;
							img_bar.oldLayout = left.layout;

							img_bar.dstStageMask = right->usage.stage;
							img_bar.dstAccessMask = right->usage.accessMask;
							img_bar.newLayout = right->layout;
							imageBarriers.push_back(img_bar);
							imageBarrierResources.push_back(
								BarrierResource<Image>{
								.resource = left.resource,
									.finalLayout = right->layout
							}
							);
						}
						//resources are unique, so no point in checking the rest if we found a match
						break;
					}
				}
			}
		}
	}

	void TaskBridge::RightOnlyBarriers(const uint8_t frameIndex, ResourceCollectionSmall& rhsColl) {
		VkImageMemoryBarrier2 img_bar{};
		img_bar.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		img_bar.pNext = nullptr;
		img_bar.srcQueueFamilyIndex = rhs->queue.family.index;
		img_bar.dstQueueFamilyIndex = rhs->queue.family.index;

		img_bar.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		img_bar.subresourceRange.baseArrayLayer = 0;
		img_bar.subresourceRange.baseMipLevel = 0;

		//layout exclusive transitions
		for (auto& rhsImage : rhsColl.images) {
			if (rhsImage->resource->layout != rhsImage->layout) {
				//add to barriers
				bool foundMatch = false;
				for (auto& existing : imageBarriers) {
					if (existing.image == rhsImage->resource->image) {
						assert(existing.newLayout == rhsImage->layout); //need to rectify this if it's an isuse
						foundMatch = true;
						break;
					}
				}
				if (foundMatch) {
					continue;
				}

				img_bar.srcQueueFamilyIndex = rhsImage->resource->owningQueue->family.index;

				img_bar.oldLayout = rhsImage->resource->layout;
				img_bar.newLayout = rhsImage->layout;
				img_bar.srcAccessMask = rhsImage->usage.accessMask; //i dont really know what to put here
				img_bar.dstAccessMask = rhsImage->usage.accessMask;
				img_bar.srcStageMask = rhsImage->usage.stage;
				img_bar.dstStageMask = rhsImage->usage.stage;
				img_bar.image = rhsImage->resource->image;
				img_bar.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				img_bar.subresourceRange.layerCount = rhsImage->resource->arrayLayers;
				img_bar.subresourceRange.levelCount = rhsImage->resource->mipLevels;

				imageBarriers.emplace_back(img_bar);
				imageBarrierResources.push_back(
					BarrierResource<Image>{
					.resource = rhsImage->resource,
						.finalLayout = rhsImage->layout
					}
				);
			}
		}

		//layout exclusive for attachments
		if (rhs->renderTracker != nullptr) {
			for (auto& col_att : rhs->renderTracker->compact.color_attachments) {
				if (col_att.imageView[frameIndex]->image.layout != VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
					auto& temp_image = col_att.imageView[frameIndex]->image;
					//add it to the barriers
					img_bar.srcQueueFamilyIndex = temp_image.owningQueue->family.index;

					img_bar.oldLayout = temp_image.layout;
					img_bar.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					img_bar.srcAccessMask = VK_ACCESS_2_NONE; //i dont really know what to put here
					img_bar.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
					img_bar.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
					img_bar.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
					img_bar.image = temp_image.image;
					img_bar.subresourceRange.layerCount = temp_image.arrayLayers;
					img_bar.subresourceRange.levelCount = temp_image.mipLevels;
					img_bar.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

					imageBarriers.emplace_back(img_bar);
					imageBarrierResources.push_back(
						BarrierResource<Image>{
						.resource = &temp_image,
							.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
					}
					);
				}
			}
			if (rhs->renderTracker->compact.depth_attachment.imageView[frameIndex] != nullptr) {
				if (rhs->renderTracker->compact.depth_attachment.imageView[frameIndex]->image.layout != VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) {
				//add to barriers
					auto& temp_image = rhs->renderTracker->compact.depth_attachment.imageView[frameIndex]->image;
					//add it to the barriers
					img_bar.srcQueueFamilyIndex = temp_image.owningQueue->family.index;

					img_bar.oldLayout = temp_image.layout;
					img_bar.newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
					img_bar.srcAccessMask = VK_ACCESS_2_NONE; //i dont really know what to put here
					img_bar.dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
					img_bar.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
					img_bar.dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT;
					img_bar.image = temp_image.image;
					img_bar.subresourceRange.layerCount = temp_image.arrayLayers;
					img_bar.subresourceRange.levelCount = temp_image.mipLevels;
					img_bar.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

					imageBarriers.emplace_back(img_bar);
					imageBarrierResources.push_back(
						BarrierResource<Image>{
						.resource = &temp_image,
						.finalLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL
						}
					);
				}
			}
		}
	}
		
	void TaskBridge::RecreateBarriers(const uint8_t frameIndex) {
		ResourceCollectionSmall rhsColl{ *rhs };

		imageBarriers.clear();
		bufferBarriers.clear();
		bufferBarrierResources.clear();
		imageBarrierResources.clear();

		if (lhs) {
			LeftToRightBarriers(frameIndex, rhsColl);
		}
		RightOnlyBarriers(frameIndex, rhsColl);
	}


	void TaskBridge::Execute(CommandBuffer& cmdBuf){
		assert(cmdBuf.commandPool.queue == rhs->queue);

		dependencyInfo.memoryBarrierCount = 0; //i could move this into the constructor but idk what this is even for tbh
		dependencyInfo.imageMemoryBarrierCount = static_cast<uint32_t>(imageBarriers.size());
		dependencyInfo.bufferMemoryBarrierCount = static_cast<uint32_t>(bufferBarriers.size());

		dependencyInfo.pMemoryBarriers = nullptr;
		dependencyInfo.pImageMemoryBarriers = imageBarriers.data();
		dependencyInfo.pBufferMemoryBarriers = bufferBarriers.data();

		const uint32_t totalSize = dependencyInfo.memoryBarrierCount + dependencyInfo.imageMemoryBarrierCount + dependencyInfo.bufferMemoryBarrierCount;
		if(totalSize > 0){
			vkCmdPipelineBarrier2(cmdBuf, &dependencyInfo);		
		}

		for (auto& ibr : imageBarrierResources) {
			ibr.resource->layout = ibr.finalLayout;
			ibr.resource->owningQueue = &rhs->queue;
		}
	}
}