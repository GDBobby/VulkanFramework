#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/Backend/Descriptor/Bindless.h"

#include "EightWinds/ImageView.h"
#include "EightWinds/Sampler.h"

#include "EightWinds/Data/StreamHelper.h"

namespace EWE{
    struct LogicalDevice;
    struct DescriptorImageInfo{

        std::string name;

        Sampler* sampler; //optional?
        ImageView& view;
        VkDescriptorImageInfo imageInfo;

        DescriptorType type;
        //im going to guarantee that this index is valid for the lifespan of this object
        DescriptorIndex index;

        [[nodiscard]] explicit DescriptorImageInfo(ImageView& view, DescriptorType type, VkImageLayout explicitLayout);
        [[nodiscard]] explicit DescriptorImageInfo(Sampler& sampler, ImageView& view, DescriptorType type, VkImageLayout explicitLayout);
        [[nodiscard]] explicit DescriptorImageInfo(ImageView& view, DescriptorType type);
        [[nodiscard]] explicit DescriptorImageInfo(Sampler& sampler, ImageView& view, DescriptorType type);
        ~DescriptorImageInfo();

        DescriptorImageInfo(DescriptorImageInfo const& copySrc) = delete;
        DescriptorImageInfo(DescriptorImageInfo&& moveSrc) noexcept;

        DescriptorImageInfo& operator=(DescriptorImageInfo const& copySrc) = delete;
        DescriptorImageInfo& operator=(DescriptorImageInfo&& moveSrc) = delete;

        operator VkDescriptorImageInfo() const{
            return imageInfo;
        }
    };
}