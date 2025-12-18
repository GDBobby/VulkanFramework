#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/LogicalDevice.h"

namespace EWE{
    struct Sampler {
        LogicalDevice& logicalDevice;
        VkSampler sampler;

        [[nodiscard]] explicit Sampler(LogicalDevice& logicalDevice, VkSamplerCreateInfo const& samplerInfo);
        ~Sampler();

        operator VkSampler() const {
            return sampler;
        }
    };
}