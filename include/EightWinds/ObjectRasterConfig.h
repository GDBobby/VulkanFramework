#pragma once

#include "EightWinds/VulkanHeader.h"

namespace EWE{
	
	struct DepthBias {
		bool  enable;
		float constantFactor;
		float clamp;
		float slopeFactor;

		bool operator==(DepthBias const& other) const noexcept {
			return enable == other.enable &&
				constantFactor == other.constantFactor &&
				clamp == other.clamp &&
				slopeFactor == other.slopeFactor;
		}
	};
	//how each object is configured
	struct ObjectRasterConfig{
		bool depthClamp;
		bool rasterizerDiscard;
		VkPolygonMode polygonMode;
		VkCullModeFlags cullMode;
		VkFrontFace frontFace;
		DepthBias depthBias;
		VkPrimitiveTopology topology;
		bool primitiveRestart;

		//i think im only going to allow 1 blend attachment max
		//if its disabled, blendAttachment.blendEnabled, it wont be added to blendStateCreateInfo
		//i think this is per object but im not sure
		VkPipelineColorBlendAttachmentState blendAttachment;

		//i need to mess with this
		float blendConstants[4];

		bool operator==(ObjectRasterConfig const& other) const noexcept {
			return
				depthClamp == other.depthClamp &&
				rasterizerDiscard == other.rasterizerDiscard &&
				polygonMode == other.polygonMode &&
				cullMode == other.cullMode &&
				frontFace == other.frontFace &&
				depthBias == other.depthBias &&
				topology == other.topology &&
				primitiveRestart == other.primitiveRestart &&

				blendAttachment.blendEnable == other.blendAttachment.blendEnable &&
				blendAttachment.srcColorBlendFactor == other.blendAttachment.srcColorBlendFactor &&
				blendAttachment.dstColorBlendFactor == other.blendAttachment.dstColorBlendFactor &&
				blendAttachment.colorBlendOp == other.blendAttachment.colorBlendOp &&
				blendAttachment.srcAlphaBlendFactor == other.blendAttachment.srcAlphaBlendFactor &&
				blendAttachment.dstAlphaBlendFactor == other.blendAttachment.dstAlphaBlendFactor &&
				blendAttachment.alphaBlendOp == other.blendAttachment.alphaBlendOp &&
				blendAttachment.colorWriteMask == other.blendAttachment.colorWriteMask &&

				blendConstants[0] == other.blendConstants[0] &&
				blendConstants[1] == other.blendConstants[1] &&
				blendConstants[2] == other.blendConstants[2] &&
				blendConstants[3] == other.blendConstants[3]
			;
		}

		void SetDefaults() noexcept;

		//VkLogicOp blendLogicOp; //need to play iwth this
	};

}