#include "EightWinds/ObjectRasterConfig.h"

namespace EWE{
	void ObjectRasterConfig::SetDefaults() noexcept {
		depthClamp = false;
		rasterizerDiscard = false;
		polygonMode = VK_POLYGON_MODE_FILL;
		cullMode = VK_CULL_MODE_NONE;
		frontFace = VK_FRONT_FACE_CLOCKWISE;
		depthBias.enable = VK_FALSE;
		depthBias.constantFactor = 0.f;
		depthBias.clamp = 0.f;
		depthBias.slopeFactor = 0.f;

		topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
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
}
