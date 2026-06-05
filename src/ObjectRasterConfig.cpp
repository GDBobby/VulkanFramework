#include "EightWinds/ObjectRasterConfig.h"

namespace EWE{


	void ObjectRasterConfig::SetDefaults() noexcept {
		depthClamp = false;
		rasterizerDiscard = false;
		cullMode = VK_CULL_MODE_NONE;
		frontFace = VK_FRONT_FACE_CLOCKWISE;
		depthBias.enable = VK_FALSE;
		depthBias.constantFactor = 0.f;
		depthBias.clamp = 0.f;
		depthBias.slopeFactor = 0.f;

		topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		polygonMode = VK_POLYGON_MODE_FILL;
		primitiveRestart = VK_FALSE;

		blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		blendAttachment.blendEnable = VK_FALSE;
		blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
		blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
		blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
		blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
		blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
		blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

		blendConstants[0] = 0.f;
		blendConstants[1] = 0.f;
		blendConstants[2] = 0.f;
		blendConstants[3] = 0.f;
	}

	bool ObjectRasterConfig::operator==(ObjectRasterConfig const& other) const noexcept {
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
}
