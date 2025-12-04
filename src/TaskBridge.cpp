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
			if (buf == nullptr) {
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
			if (img == nullptr) {
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
			}
			for (auto& push : task.pushTrackers) {
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

	void TaskBridge::RecreateBarriers() {
		ResourceCollectionSmall lhsColl{ lhs };
		ResourceCollectionSmall rhsColl{ rhs };

		imageBarriers.clear();
		bufferBarriers.clear();

		{ //buffers
			VkBufferMemoryBarrier2 buf_bar{};
			buf_bar.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
			buf_bar.pNext = nullptr;
			buf_bar.srcQueueFamilyIndex = lhs.queue.family.index;
			buf_bar.dstQueueFamilyIndex = rhs.queue.family.index;

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
						//buffers are unique, so no point in checking the rest if we found a match
						break;
					}
				}
			}
		}

		{ //images
			VkImageMemoryBarrier2 img_bar{};
			img_bar.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
			img_bar.pNext = nullptr;
			img_bar.srcQueueFamilyIndex = lhs.queue.family.index;
			img_bar.dstQueueFamilyIndex = rhs.queue.family.index;

			img_bar.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			img_bar.subresourceRange.baseArrayLayer = 0;
			img_bar.subresourceRange.baseMipLevel = 0;

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
							img_bar.oldLayout = left->resource->layout;

							img_bar.dstStageMask = right->usage.stage;
							img_bar.dstAccessMask = right->usage.accessMask;
							img_bar.newLayout = right->resource->layout;
							imageBarriers.push_back(img_bar);
						}
						//buffers are unique, so no point in checking the rest if we found a match
						break;
					}
				}
			}
		}

		{ //attachments	
			std::vector<Resource<Image>> previousAttachments{};

			VkImageMemoryBarrier2 img_bar{};
			img_bar.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
			img_bar.pNext = nullptr;
			img_bar.srcQueueFamilyIndex = lhs.queue.family.index;
			img_bar.dstQueueFamilyIndex = rhs.queue.family.index;

			img_bar.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			img_bar.subresourceRange.baseArrayLayer = 0;
			img_bar.subresourceRange.baseMipLevel = 0;

			if(lhs.renderTracker != nullptr) {
				std::size_t totalAttachmentCount = lhs.renderTracker->compact.color_attachments.size();
				const bool hasDepth = lhs.renderTracker->compact.depth_attachment.imageView != nullptr;
				totalAttachmentCount += hasDepth;

				previousAttachments.reserve(totalAttachmentCount);
				for(auto& col_att : lhs.renderTracker->compact.color_attachments){
					auto& backAtt = previousAttachments.emplace_back();
					backAtt.resource = &col_att.imageView->image;
					backAtt.usage.stage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
					backAtt.usage.accessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
				} 
				{ //depth
					auto& backAtt = previousAttachments.emplace_back();
					if(hasDepth){ 
						backAtt.resource = &lhs.renderTracker->compact.depth_attachment.imageView->image;
						/*
							VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT specifies the stage of the pipeline where early fragment tests 
							(depth and stencil tests before fragment shading) are performed. This stage also includes render pass load 
							operations for framebuffer attachments with a depth/stencil format.
						*/
						backAtt.usage.stage = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT;
						backAtt.usage.accessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
					} 
				}
			}

			for(auto& left : previousAttachments){
			//now we find matches
				for (auto* right : rhsColl.images) {
					if (left.resource == right->resource) {
						bool leftWrites = GetAccessMaskWrite(left.usage.accessMask);
						bool rightWrites = GetAccessMaskWrite(right->usage.accessMask);

						if (leftWrites || rightWrites) {
							img_bar.image = left.resource->image;

							img_bar.subresourceRange.layerCount = left.resource->arrayLayers;
							img_bar.subresourceRange.levelCount = left.resource->mipLevels;

							img_bar.srcStageMask = left.usage.stage;
							img_bar.srcAccessMask = left.usage.accessMask;
							img_bar.oldLayout = left.resource->layout;

							img_bar.dstStageMask = right->usage.stage;
							img_bar.dstAccessMask = right->usage.accessMask;
							img_bar.newLayout = right->resource->layout;
							imageBarriers.push_back(img_bar);
						}
						//buffers are unique, so no point in checking the rest if we found a match
						break;
					}
				}
			}
		}
	}


	void TaskBridge::Submit(CommandBuffer& cmdBuf){
		assert(cmdBuf.commandPool.queue == rhs.queue);

		dependencyInfo.memoryBarrierCount = 0;
		dependencyInfo.imageMemoryBarrierCount = static_cast<uint32_t>(imageBarriers.size());
		dependencyInfo.bufferMemoryBarrierCount = static_cast<uint32_t>(bufferBarriers.size());

		dependencyInfo.pMemoryBarriers = nullptr;
		dependencyInfo.pImageMemoryBarriers = imageBarriers.data();
		dependencyInfo.pBufferMemoryBarriers = bufferBarriers.data();

		const uint32_t totalSize = dependencyInfo.memoryBarrierCount + dependencyInfo.imageMemoryBarrierCount + dependencyInfo.bufferMemoryBarrierCount;
		if(totalSize > 0){
			vkCmdPipelineBarrier2(cmdBuf, &dependencyInfo);		
		}
	}
}