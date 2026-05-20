#pragma once

#include "EightWinds/VulkanHeader.h"

#include "EightWinds/Data/PerFlight.h"
#include "EightWinds/Data/RuntimeArray.h"
#include "EightWinds/Command/InstructionPointer.h"
#include "EightWinds/Command/InstructionType.h"
#include "EightWinds/Command/ParamPacks.h"
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

		void Init(RenderAttachments const& attachments, VkRenderingFlags renderingFlags, uint32_t screen_width, uint32_t screen_height);
		[[nodiscard]] Render_Vk_Data() = default;
		[[nodiscard]] explicit Render_Vk_Data(RenderAttachments const& attachments, uint32_t screen_width, uint32_t screen_height);
	};

    struct AttachmentInfo {
        VkFormat                format;
        VkAttachmentLoadOp      loadOp;
        VkAttachmentStoreOp     storeOp;
        VkClearValue            clearValue;
    };

    struct AttachmentSetInfo {
		
		bool relative_size = true;
        float width;
        float height;
		VkRenderingFlags renderingFlags{ 0 };
        RuntimeArray<AttachmentInfo> colors{0};
		bool using_depth = true;
        AttachmentInfo depth;

		[[nodiscard]] AttachmentSetInfo() = default;
		[[nodiscard]] AttachmentSetInfo(
			uint32_t _width, uint32_t _height, 
			VkRenderingFlags _renderingFlags,
        	std::span<const AttachmentInfo> _colors,
        	AttachmentInfo _depth 
		);
		[[nodiscard]] AttachmentSetInfo(
			uint32_t _width, uint32_t _height, 
			VkRenderingFlags _renderingFlags,
        	std::span<const AttachmentInfo> _colors
		);

		[[nodiscard]] AttachmentSetInfo(AttachmentSetInfo const& copySrc);

		AttachmentSetInfo& operator=(AttachmentSetInfo const& copySrc);

		VkRect2D CalculateRenderArea(uint32_t screen_width, uint32_t screen_height) const;
    };

	struct FullRenderInfo; //forward declare

	struct AttachmentMeta{
		/*
			a color view can be generated in a different render attachment object. it can be referenced.
			this tracks those references, specifically for cross-runtime stability

			if the owner is this, the attachment will be generated

			if the owner is nullptr, the image view will not be generated, and will be left null
			the programmer is responsible for ensuring it gets populated
			this can be useful for putting an arbitrary image into the attachment slot (like the present image)
		*/

		FullRenderInfo* src_owner; //if source_owner is nullptr, 'this' is the owner
		int8_t src_index;
	};

	struct RenderAttachments {
		std::filesystem::path name;
		LogicalDevice& logicalDevice;
		Queue& graphicsQueue;

		AttachmentSetInfo setInfo;

		RuntimeArray<AttachmentMeta> meta;

		RuntimeArray<PerFlight<ImageView*>> color_views;
		PerFlight<ImageView*> depth_views;


		[[nodiscard]] explicit RenderAttachments(
			std::filesystem::path const& name,
			LogicalDevice& logicalDevice,
			Queue& graphicsQueue,
			AttachmentSetInfo const& setInfo,
			uint32_t width, uint32_t height
		);
		[[nodiscard]] explicit RenderAttachments(
			std::filesystem::path const& name,
			LogicalDevice& logicalDevice,
			Queue& graphicsQueue,
			AttachmentSetInfo const& setInfo,
			std::span<const AttachmentMeta> _meta,
			uint32_t width, uint32_t height
		);

		//use negative index to generate depth
		void GenerateImage(
			PerFlight<Image*> img_con_addr, 
			PerFlight<ImageView*> view_con_addr, 
			uint32_t width, uint32_t height,
			int index
		); 

		void CreateImages(uint32_t width, uint32_t height);
		void InitialTransition();
	};


	struct FullRenderInfo {
		RenderAttachments full;
		Render_Vk_Data render_data;
		std::filesystem::path& name; //reference to full.name

		[[nodiscard]] explicit FullRenderInfo(
			std::filesystem::path const& name,
			LogicalDevice& logicalDevice,
			Queue& graphicsQueue,
			AttachmentSetInfo const& setInfo,
			uint32_t width, uint32_t height
		);
		[[nodiscard]] explicit FullRenderInfo(
			std::filesystem::path const& name,
			LogicalDevice& logicalDevice, Queue& graphicsQueue,
			AttachmentSetInfo const& setInfo,
			std::span<const AttachmentMeta> _meta,
			uint32_t width, uint32_t height
		);

		void Undefer(InstructionPointer<ParamPack<Inst::BeginRender>>* deferred_render_info);
	};
} //namespace EWE