#pragma once

#include "EightWinds/VulkanHeader.h"

#include "EightWinds/Data/PerFlight.h"
#include "EightWinds/Data/RuntimeArray.h"
#include "EightWinds/RenderGraph/Command/DeferredReference.h"
#include "EightWinds/Image.h"
#include "EightWinds/ImageView.h"

#include "EightWinds/Backend/RenderInfo.h"

#include "EightWinds/DescriptorImageInfo.h"


#include <vector>


namespace EWE{

	struct RenderAttachments;

	struct RenderInfo {
		std::vector<VkRenderingAttachmentInfo> colors;
		VkRenderingAttachmentInfo depth;

		[[nodiscard]] explicit RenderInfo(RenderAttachments const& attachments, uint8_t frameIndex);
	};

	struct Render_Vk_Data {
		PerFlight<RenderInfo> vk_data;
		PerFlight<VkRenderingInfo> vk_info;

		[[nodiscard]] explicit Render_Vk_Data(RenderAttachments const& attachments, VkRenderingFlags renderingFlags);
	};

    struct AttachmentInfo {
        VkFormat                format;
        VkAttachmentLoadOp      loadOp;
        VkAttachmentStoreOp     storeOp;
        VkClearValue            clearValue;
    };

    struct AttachmentSetInfo {
        uint32_t width;
        uint32_t height;
		VkRenderingFlags renderingFlags{ 0 };
        std::vector<AttachmentInfo> colors;
        AttachmentInfo depth; //should be optional

		VkRect2D CalculateRenderArea() const;
    };

	struct RenderAttachments {
		LogicalDevice& logicalDevice;
		Queue& graphicsQueue;
		const std::string name;

		[[nodiscard]] explicit RenderAttachments(
			std::string_view name,
			LogicalDevice& logicalDevice,
			Queue& graphicsQueue,
			AttachmentSetInfo const& setInfo
		);

		RuntimeArray<PerFlight<Image>> color_images;
		HeapBlock<PerFlight<ImageView>> color_views; //i want this to be a runtimearray but i need to delay the construction
		//these should be optional
		PerFlight<Image> depth_images;
		HeapBlock<PerFlight<ImageView>> depth_views; //singular, but the alternative is to just use a pointer which is somewhat misleading

		AttachmentSetInfo setInfo; //do I even need this here?

		void CreateImages(std::string_view name, uint32_t width, uint32_t height, AttachmentSetInfo const& setInfo);
		void CreateImageViews();
		void InitialTransition();
	};


	struct FullRenderInfo {
		RenderAttachments full;
		Render_Vk_Data render_data;

		[[nodiscard]] explicit FullRenderInfo(
			std::string_view name,
			LogicalDevice& logicalDevice,
			Queue& graphicsQueue,
			AttachmentSetInfo const& setInfo
		);

		DeferredReference<VkRenderingInfo> deferred_render_info;

		void Undefer();
	};
} //namespace EWE