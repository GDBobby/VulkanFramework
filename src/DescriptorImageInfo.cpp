#include "EightWinds/DescriptorImageInfo.h"

namespace EWE{
    DescriptorImageInfo::DescriptorImageInfo(LogicalDevice& logicalDevice, ImageView& view, DescriptorType type, VkImageLayout explicitLayout)
        : logicalDevice{ logicalDevice },
        sampler{ nullptr },
        view{ view },
        imageInfo{
            .sampler = VK_NULL_HANDLE,
            .imageView = view,
            .imageLayout = explicitLayout
        },
        type{ type },
        index{ logicalDevice.bindlessDescriptor.BindImage(imageInfo, type) }
    {
    }
    DescriptorImageInfo::DescriptorImageInfo(LogicalDevice& logicalDevice, Sampler& sampler, ImageView& view, DescriptorType type, VkImageLayout explicitLayout)
        : logicalDevice{ logicalDevice },
        sampler{ &sampler },
        view{ view },
        imageInfo{
            .sampler = sampler,
            .imageView = view,
            .imageLayout = explicitLayout
        },
        type{ type },
        index{ logicalDevice.bindlessDescriptor.BindImage(imageInfo, type) }
    {
    }
    DescriptorImageInfo::DescriptorImageInfo(LogicalDevice& logicalDevice, ImageView& view, DescriptorType type)
        : DescriptorImageInfo(logicalDevice, view, type, view.image.layout)
    {
    }
    DescriptorImageInfo::DescriptorImageInfo(LogicalDevice& logicalDevice, Sampler& sampler, ImageView& view, DescriptorType type)
        : DescriptorImageInfo(logicalDevice, sampler, view, type, view.image.layout)
    {
    }

    DescriptorImageInfo::DescriptorImageInfo(DescriptorImageInfo&& moveSrc) noexcept
        : logicalDevice{ moveSrc.logicalDevice },
        sampler{ moveSrc.sampler },
        view{ moveSrc.view },
        imageInfo{moveSrc.imageInfo},
        type{ moveSrc.type },
        index{ moveSrc.index }
    {
        moveSrc.index = INVALID_DESCRIPTOR_INDEX;
    }

    DescriptorImageInfo::~DescriptorImageInfo() {
        //if it was moved, index will be invalid
        if (index != INVALID_DESCRIPTOR_INDEX) {
            logicalDevice.bindlessDescriptor.Unbind(index, type);
        }
    }
}