#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/Image.h"
#include "EightWinds/ImageView.h"

#include "EightWinds/Backend/RenderInfo.h"

#include "EightWinds/DescriptorImageInfo.h"

#include "EightWinds/Data/PerFlight.h"

#include "EightWinds/Data/RuntimeArray.h"

#include <vector>

namespace EWE{
	

	struct AttachmentConstructionInfo {
		VkFormat format;
		AttachmentInfo info;
	};

	struct RenderInfo3 {
		LogicalDevice& logicalDevice;
		Queue& graphicsQueue;

		const std::string name;

		[[nodiscard]] explicit RenderInfo3(
			std::string_view name,
			LogicalDevice& logicalDevice,
			Queue& graphicsQueue,
			uint32_t width, uint32_t height,
			std::vector<AttachmentConstructionInfo> const& color_infos,
			AttachmentConstructionInfo depth_info
		);
		
		//if i make this perflight<array< then i can easily get the full image data, 
		//but if i do array<perflight< i can easily operate on each pair within the array
		RuntimeArray<PerFlight<Image>> color_images;
		RuntimeArray<PerFlight<ImageView>> color_views;
		
		//these are optional
		PerFlight<Image> depth_images;
		PerFlight<ImageView> depth_views;
		
		
		//VkFormat colorFormat = VK_FORMAT_R8G8B8A8_UNORM;
		std::vector<VkFormat> color_formats;
		VkFormat depth_format = VK_FORMAT_D16_UNORM;
		
		//descriptors?

		//other stuff should be put here

		void CreateImages(std::string_view name, uint32_t width, uint32_t height, std::vector<AttachmentConstructionInfo> const& color_infos);
		
		//void ApplyToParamBuffer(DeferredReference<VkRenderingInfo>& buffer);

		void InitialTransition();
		
	};
}