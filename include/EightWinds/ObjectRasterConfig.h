#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/VulkanHash.h"

#include "EightWinds/Data/RuntimeArray.h"

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

		RuntimeArray<VkPipelineColorBlendAttachmentState> blendAttachments;
		
		float blendConstants[4]; //i need to mess with this

		[[nodiscard]] ObjectRasterConfig() = default;
		[[nodiscard]] ObjectRasterConfig(ObjectRasterConfig const& copySrc);
		ObjectRasterConfig& operator=(ObjectRasterConfig const& copySrc);

		bool operator==(ObjectRasterConfig const& other) const noexcept;

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
		;
		for(auto& ba : c.blendAttachments){
			hashCombiner.Combine(std::hash<VkPipelineColorBlendAttachmentState>{}(ba));
		}

		for (float f : c.blendConstants) {
			hashCombiner.Combine(std::hash<float>{}(f));
		}
		return hashCombiner.seed;
	}
};
