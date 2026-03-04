#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/LogicalDevice.h"

namespace EWE{
    struct Sampler {
        LogicalDevice& logicalDevice;
        VkSamplerCreateInfo const& samplerInfo;
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
    };
}