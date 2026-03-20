#include "EightWinds/DescriptorImageInfo.h"



namespace EWE{


    DescriptorImageInfo::DescriptorImageInfo(ImageView& view, DescriptorType type, VkImageLayout explicitLayout)
        : sampler{ nullptr },
        view{ view },
        imageInfo{
            .sampler = VK_NULL_HANDLE,
            .imageView = view,
            .imageLayout = explicitLayout
        },
        type{ type },
        index{ view.image.logicalDevice.bindlessDescriptor.BindImage(imageInfo, type) }
    {
    }
    DescriptorImageInfo::DescriptorImageInfo(Sampler& sampler, ImageView& view, DescriptorType type, VkImageLayout explicitLayout)
        : sampler{ &sampler },
        view{ view },
        imageInfo{
            .sampler = sampler,
            .imageView = view,
            .imageLayout = explicitLayout
        },
        type{ type },
        index{ view.image.logicalDevice.bindlessDescriptor.BindImage(imageInfo, type) }
    {
    }
    DescriptorImageInfo::DescriptorImageInfo(ImageView& view, DescriptorType type)
        : DescriptorImageInfo(view, type, view.image.data.layout)
    {
    }
    DescriptorImageInfo::DescriptorImageInfo(Sampler& sampler, ImageView& view, DescriptorType type)
        : DescriptorImageInfo(sampler, view, type, view.image.data.layout)
    {
    }

    DescriptorImageInfo::DescriptorImageInfo(DescriptorImageInfo&& moveSrc) noexcept
        : sampler{ moveSrc.sampler },
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
            view.image.logicalDevice.bindlessDescriptor.Unbind(index, type);
        }
    }
}