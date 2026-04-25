#pragma once

#include "EightWinds/VulkanHeader.h"

#include "EightWinds/Data/PerFlight.h"
#include "EightWinds/Data/RuntimeArray.h"
#include "EightWinds/Command/InstructionPointer.h"
#include "EightWinds/Image.h"
#include "EightWinds/ImageView.h"

#include "EightWinds/Backend/RenderInfo.h"

#include "EightWinds/DescriptorImageInfo.h"


#include <vector>
#include <span>


namespace EWE{

	struct RenderAttachments;

	struct RenderInfo {
		std::vector<VkRenderingAttachmentInfo> colors;
		VkRenderingAttachmentInfo depth;

		void Init(RenderAttachments const& attachments, uint8_t frameIndex);
		[[nodiscard]] RenderInfo() = default;
		[[nodiscard]] explicit RenderInfo(RenderAttachments const& attachments, uint8_t frameIndex);
	};

	struct Render_Vk_Data {
		PerFlight<RenderInfo> vk_data;
		PerFlight<VkRenderingInfo> vk_info;

		void Init(RenderAttachments const& attachments, VkRenderingFlags renderingFlags);
		[[nodiscard]] Render_Vk_Data() = default;
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
        RuntimeArray<AttachmentInfo> colors{0};
        AttachmentInfo depth; //should be optional

		[[nodiscard]] AttachmentSetInfo() = default;
		[[nodiscard]] AttachmentSetInfo(
			uint32_t _width, uint32_t _height, 
			VkRenderingFlags _renderingFlags,
        	std::span<const AttachmentInfo> _colors,
        	AttachmentInfo _depth 
		);

		[[nodiscard]] AttachmentSetInfo(AttachmentSetInfo const& copySrc);

		VkRect2D CalculateRenderArea() const;
    };

	struct RenderAttachments {
		const std::string name;
		LogicalDevice& logicalDevice;
		Queue& graphicsQueue;

		RuntimeArray<PerFlight<Image*>> color_images;
		RuntimeArray<PerFlight<ImageView*>> color_views;
		//these should be optional
		PerFlight<Image*> depth_image;
		PerFlight<ImageView*> depth_views;

		AttachmentSetInfo setInfo; //do I even need this here?

		[[nodiscard]] explicit RenderAttachments(
			std::string_view name,
			LogicalDevice& logicalDevice,
			Queue& graphicsQueue,
			AttachmentSetInfo const& setInfo
		);

		void CreateImages(uint32_t width, uint32_t height);
		void CreateImageViews();
		void InitialTransition();

		void Init();
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

		void Init();

		void Undefer(InstructionPointer<VkRenderingInfo>* deferred_render_info);
	};
} //namespace EWE