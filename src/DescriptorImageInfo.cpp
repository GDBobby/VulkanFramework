#include "EightWinds/DescriptorImageInfo.h"



namespace EWE{


    DescriptorImageInfo::DescriptorImageInfo(ImageView& _view, DescriptorType _type, VkImageLayout explicitLayout)
        : sampler{ nullptr },
        view{ _view },
        imageInfo{
            .sampler = VK_NULL_HANDLE,
            .imageView = view,
            .imageLayout = explicitLayout
        },
        type{ _type },
        index{ view.image.logicalDevice.bindlessDescriptor.BindImage(imageInfo, type) }
    {
    }
    DescriptorImageInfo::DescriptorImageInfo(Sampler& _sampler, ImageView& _view, DescriptorType _type, VkImageLayout explicitLayout)
        : sampler{ &_sampler },
        view{ _view },
        imageInfo{
            .sampler = *sampler,
            .imageView = view,
            .imageLayout = explicitLayout
        },
        type{ _type },
        index{ view.image.logicalDevice.bindlessDescriptor.BindImage(imageInfo, type) }
    {
    }
    DescriptorImageInfo::DescriptorImageInfo(ImageView& _view, DescriptorType _type)
        : DescriptorImageInfo(_view, _type, view.image.data.layout)
    {
    }
    DescriptorImageInfo::DescriptorImageInfo(Sampler& _sampler, ImageView& _view, DescriptorType _type)
        : DescriptorImageInfo(_sampler, _view, _type, view.image.data.layout)
    {
    }

    DescriptorImageInfo::DescriptorImageInfo(DescriptorImageInfo&& moveSrc) noexcept
        : sampler{ moveSrc.sampler },
        view{ moveSrc.view },
        imageInfo{moveSrc.imageInfo},
        type{ moveSrc.type },
        index{ moveSrc.index }
    {
        moveSrc.index = null_texture;
    }

    DescriptorImageInfo::~DescriptorImageInfo() {
        //if it was moved, index will be invalid
        if (index != null_texture) {
            view.image.logicalDevice.bindlessDescriptor.Unbind(index, type);
        }
    }
}