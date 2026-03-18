#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/LogicalDevice.h"

#include <cstdint>

namespace EWE{
    struct Sampler {
        LogicalDevice& logicalDevice;
        const VkSamplerCreateInfo info;
        VkSampler sampler;

        [[nodiscard]] explicit Sampler(LogicalDevice& logicalDevice, VkSamplerCreateInfo const& samplerInfo);
        ~Sampler();

        Sampler(Sampler const& copySrc) = delete;
        Sampler(Sampler&& moveSrc) = delete;
        Sampler& operator=(Sampler const& copySrc) = delete;
        Sampler& operator=(Sampler&& moveSrc) = delete;

        operator VkSampler() const {
            return sampler;
        }

        static uint64_t Condense(VkSamplerCreateInfo const& info);
        static VkSamplerCreateInfo Expand(uint64_t condensed);
    };
}