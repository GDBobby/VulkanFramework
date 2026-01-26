#pragma once

#include <cstdint>
#include <functional>

#include "EightWinds/VulkanHeader.h"

namespace EWE{
	static constexpr std::size_t vulkan_hash_val = 0x9E3779B97F4A7C15;

	inline void hash_combine(std::size_t& seed, std::size_t value) noexcept {
		seed ^= value + vulkan_hash_val + (seed << 6) + (seed >> 2);
	}
	
	struct HashCombiner {
		std::size_t seed{ 0 };
		constexpr HashCombiner& Combine(std::size_t value) { seed ^= value + vulkan_hash_val + (seed << 6) + (seed >> 2); return *this; }
	};
}

//template<>
//struct std::hash<VkBool32> {
//	std::size_t operator()(VkBool32 b) const noexcept {
//		return std::hash<bool>{}(b != 0);
//	}
//};

template<>
struct std::hash<VkBlendFactor> {
	std::size_t operator()(VkBlendFactor f) const noexcept {
		const uint8_t val = static_cast<uint8_t>(f);
#if EWE_DEBUG_BOOL
		assert(val != VK_BLEND_FACTOR_MAX_ENUM);
#endif
		return std::hash<uint8_t>{}(static_cast<uint8_t>(val));
	}
};

template<>
struct std::hash<VkBlendOp> {
	std::size_t operator()(VkBlendOp op) const noexcept {
		const uint8_t val = static_cast<uint8_t>(op);
#if EWE_DEBUG_BOOL
		assert(val != VK_BLEND_OP_MAX_ENUM);
#endif
		return std::hash<uint8_t>{}(static_cast<uint8_t>(val));
	}
};

template<>
struct std::hash<VkColorComponentFlags> {
	std::size_t operator()(VkColorComponentFlags flags) const noexcept {
		const uint8_t val = static_cast<uint8_t>(flags);
#if EWE_DEBUG_BOOL
		assert(val != VK_COLOR_COMPONENT_FLAG_BITS_MAX_ENUM);
#endif
		return std::hash<uint8_t>{}(static_cast<uint8_t>(val));
	}
};

template<>
struct std::hash<VkPipelineColorBlendAttachmentState> {
	std::size_t operator()(const VkPipelineColorBlendAttachmentState& s) const noexcept {

		EWE::HashCombiner hashCombiner{};
		hashCombiner.Combine(std::hash<VkBool32>{}(s.blendEnable))
			.Combine(std::hash<VkBlendFactor>{}(s.srcColorBlendFactor))
			.Combine(std::hash<VkBlendFactor>{}(s.dstColorBlendFactor))
			.Combine(std::hash<VkBlendOp>{}(s.colorBlendOp))
			.Combine(std::hash<VkBlendFactor>{}(s.srcAlphaBlendFactor))
			.Combine(std::hash<VkBlendFactor>{}(s.dstAlphaBlendFactor))
			.Combine(std::hash<VkBlendOp>{}(s.alphaBlendOp))
			.Combine(std::hash<VkColorComponentFlags>{}(s.colorWriteMask));
		return hashCombiner.seed;
	}
};