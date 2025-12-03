#include "EightWinds/RenderGraph/TaskBridge.h"

#include "EightWinds/ImageView.h"

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
			if (task.renderTracker != nullptr) {
					//compact needs to be changed to use resources instead of raw image pointers?
				for (auto& col_att : task.renderTracker->compact.color_attachments) {
					assert(col_att.imageView != nullptr);
					AddUniqueImage(&col_att.imageView->image);
				}
			}
		}
	};

	void TaskBridge::CreateBarriers() {
		ResourceCollectionSmall lhsColl{ lhs };
		ResourceCollectionSmall rhsColl{ rhs };

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
							//theres probably more to the aspect mask
							//if its depth AND stencil, the apsect must be both depth and stenci lbit
							if (left->resource->usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
								img_bar.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
							}
							else {
								img_bar.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
							}

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

	}
}