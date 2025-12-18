#include "EightWinds/Sampler.h"

namespace EWE{
        Sampler::Sampler(LogicalDevice& logicalDevice, VkSamplerCreateInfo const& samplerInfo)
            : logicalDevice{logicalDevice}
        {
            EWE_VK(vkCreateSampler, logicalDevice, &samplerInfo, nullptr, &sampler);
        }

        Sampler::~Sampler() {
            logicalDevice.garbageDisposal.Toss(sampler, VK_OBJECT_TYPE_SAMPLER);
        }
}