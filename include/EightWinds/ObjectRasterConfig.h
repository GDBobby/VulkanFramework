#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/VulkanHash.h"

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
		//i think im only going to allow 1 blend attachment max
		//if its disabled, blendAttachment.blendEnabled, it wont be added to blendStateCreateInfo
		//i think this is per object but im not sure
	struct ObjectRasterConfig{
		bool depthClamp;
		bool rasterizerDiscard;
		VkPolygonMode polygonMode;
		VkCullModeFlags cullMode;
		VkFrontFace frontFace;
		DepthBias depthBias;
		VkPrimitiveTopology topology;
		bool primitiveRestart;

		VkPipelineColorBlendAttachmentState blendAttachment;
		
		float blendConstants[4]; //i need to mess with this

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

} //namespace EWE

template<>
struct std::hash<EWE::DepthBias> {
	std::size_t operator()(EWE::DepthBias const& d) const noexcept {
		EWE::HashCombiner hashCombiner{};
		hashCombiner.Combine(std::hash<bool>{}(d.enable))
			.Combine(std::hash<float>{}(d.constantFactor))
			.Combine(std::hash<float>{}(d.clamp))
			.Combine(std::hash<float>{}(d.slopeFactor));
		return hashCombiner.seed;
	}
};
template<>
struct std::hash<EWE::ObjectRasterConfig> {
	std::size_t operator()(EWE::ObjectRasterConfig const& c) const noexcept {
		EWE::HashCombiner hashCombiner{};
		hashCombiner.Combine(std::hash<bool>{}(c.depthClamp))
					.Combine(std::hash<bool>{}(c.rasterizerDiscard))
					.Combine(std::hash<int>{}(c.polygonMode))
					.Combine(std::hash<int>{}(c.cullMode))
					.Combine(std::hash<int>{}(c.frontFace))
					.Combine(std::hash<EWE::DepthBias>{}(c.depthBias))
					.Combine(std::hash<int>{}(c.topology))
					.Combine(std::hash<bool>{}(c.primitiveRestart))
					.Combine(std::hash<VkPipelineColorBlendAttachmentState>{}(c.blendAttachment))
		;

		for (float f : c.blendConstants) {
			hashCombiner.Combine(std::hash<float>{}(f));
		}
		return hashCombiner.seed;
	}
};
