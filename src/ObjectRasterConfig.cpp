#include "EightWinds/ObjectRasterConfig.h"

namespace EWE{

	ObjectRasterConfig::ObjectRasterConfig(ObjectRasterConfig const& copySrc)
	: depthClamp{copySrc.depthClamp},
		rasterizerDiscard{copySrc.rasterizerDiscard},
		polygonMode{copySrc.polygonMode},
		cullMode{copySrc.cullMode},
		frontFace{copySrc.frontFace},
		depthBias{copySrc.depthBias},
		topology{copySrc.topology},
		primitiveRestart{copySrc.primitiveRestart},
		blendAttachments{copySrc.blendAttachments.Size()}
	{

		for(std::size_t i = 0; i < blendAttachments.Size(); i++){
			blendAttachments[i] = copySrc.blendAttachments[i];
		}
		memcpy(blendConstants, copySrc.blendConstants, sizeof(float) * 4);
	}

	ObjectRasterConfig& ObjectRasterConfig::operator=(ObjectRasterConfig const& copySrc){
		depthClamp = copySrc.depthClamp;
		rasterizerDiscard = copySrc.rasterizerDiscard;
		polygonMode = copySrc.polygonMode;
		cullMode = copySrc.cullMode;
		frontFace = copySrc.frontFace;
		depthBias = copySrc.depthBias;
		topology = copySrc.topology;
		primitiveRestart = copySrc.primitiveRestart;
		blendAttachments.ClearAndResize(copySrc.blendAttachments.Size());
		for(std::size_t i = 0; i < blendAttachments.Size(); i++){
			blendAttachments[i] = copySrc.blendAttachments[i];
		}
		memcpy(blendConstants, copySrc.blendConstants, sizeof(float) * 4);
		return *this;
	}

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

		blendAttachments.ClearAndResize(1);
		blendAttachments[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		blendAttachments[0].blendEnable = VK_FALSE;
		blendAttachments[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;   // Optional
		blendAttachments[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;  // Optional
		blendAttachments[0].colorBlendOp = VK_BLEND_OP_ADD;              // Optional
		blendAttachments[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
		blendAttachments[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
		blendAttachments[0].alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

		blendConstants[0] = 0.f;
		blendConstants[1] = 0.f;
		blendConstants[2] = 0.f;
		blendConstants[3] = 0.f;
	}

	bool ObjectRasterConfig::operator==(ObjectRasterConfig const& other) const noexcept {

		if(blendAttachments.Size() != other.blendAttachments.Size()){
			return false;
		}

		for(std::size_t i = 0; i < blendAttachments.Size(); i++){
			const bool matching = 
			blendAttachments[i].blendEnable 		== other.blendAttachments[i].blendEnable &&
			blendAttachments[i].srcColorBlendFactor == other.blendAttachments[i].srcColorBlendFactor &&
			blendAttachments[i].dstColorBlendFactor == other.blendAttachments[i].dstColorBlendFactor &&
			blendAttachments[i].colorBlendOp 		== other.blendAttachments[i].colorBlendOp &&
			blendAttachments[i].srcAlphaBlendFactor == other.blendAttachments[i].srcAlphaBlendFactor &&
			blendAttachments[i].dstAlphaBlendFactor == other.blendAttachments[i].dstAlphaBlendFactor &&
			blendAttachments[i].alphaBlendOp 		== other.blendAttachments[i].alphaBlendOp &&
			blendAttachments[i].colorWriteMask 		== other.blendAttachments[i].colorWriteMask;
			if(!matching){
				return false;
			}
		}

		return
			depthClamp == other.depthClamp &&
			rasterizerDiscard == other.rasterizerDiscard &&
			polygonMode == other.polygonMode &&
			cullMode == other.cullMode &&
			frontFace == other.frontFace &&
			depthBias == other.depthBias &&
			topology == other.topology &&
			primitiveRestart == other.primitiveRestart &&

			blendConstants[0] == other.blendConstants[0] &&
			blendConstants[1] == other.blendConstants[1] &&
			blendConstants[2] == other.blendConstants[2] &&
			blendConstants[3] == other.blendConstants[3]
		;
	}
}
