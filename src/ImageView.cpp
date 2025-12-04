#include "EightWinds/ImageView.h"

namespace EWE{



    constexpr VkImageViewType ImageTypeToViewType(VkImageType imageType, uint32_t arrayCount) {
        //https://docs.vulkan.org/refpages/latest/refpages/source/VkImageViewCreateInfo.html
        //conversion chart here
        switch(imageType) {
            case VK_IMAGE_TYPE_1D:
                if(arrayCount > 1){
                    return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
                }
                return VK_IMAGE_VIEW_TYPE_1D;
            case VK_IMAGE_TYPE_2D:
                if(arrayCount == 6){
                    return VK_IMAGE_VIEW_TYPE_CUBE;
                }
                if(arrayCount > 1){
                    return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
                }
                return VK_IMAGE_VIEW_TYPE_2D;
            case VK_IMAGE_TYPE_3D:
                return VK_IMAGE_VIEW_TYPE_3D;

            default:
                //cube array usage?
                assert(false);
                return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
        }
    }


    VkImageSubresourceRange ImageView::GetDefaultSubresource(Image& image) noexcept{

        VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        switch (image.format) {
            case VK_FORMAT_D16_UNORM:
            case VK_FORMAT_D16_UNORM_S8_UINT:
            case VK_FORMAT_D24_UNORM_S8_UINT:
            case VK_FORMAT_D32_SFLOAT:
            case VK_FORMAT_D32_SFLOAT_S8_UINT:
            case VK_FORMAT_X8_D24_UNORM_PACK32:
                aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
                break;
            default: 
                break;
        }
        

        VkImageSubresourceRange ret;
        ret.aspectMask = aspectMask;
        ret.baseArrayLayer = 0;
        ret.baseMipLevel = 0;
        ret.layerCount = image.arrayLayers;
        ret.levelCount = image.mipLevels;

        return ret;
    }

    VkImageViewCreateInfo ImageView::GetDefaultFullImageViewCreateInfo(Image& image) noexcept {
        VkImageViewCreateInfo viewCreateInfo{};
        viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewCreateInfo.pNext = nullptr;
        /*
        VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DYNAMIC_BIT_EXT = 0x00000001,
        // Provided by VK_EXT_descriptor_buffer
        VK_IMAGE_VIEW_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT = 0x00000004,
        // Provided by VK_EXT_fragment_density_map2
        VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DEFERRED_BIT_EXT = 0x00000002,
        */
        viewCreateInfo.flags = 0;
        viewCreateInfo.image = image.image;

        //i think i need a function converting the type, but its simple
        //theres a conversion chart on that page
        viewCreateInfo.viewType = ImageTypeToViewType(image.type, image.arrayLayers);
        viewCreateInfo.format = image.format;
        viewCreateInfo.components = VkComponentMapping{
                                        .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                                        .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                                        .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                                        .a = VK_COMPONENT_SWIZZLE_IDENTITY
        };

        viewCreateInfo.subresourceRange = GetDefaultSubresource(image);

        return viewCreateInfo;
    }

    ImageView::ImageView(Image& image, VkImageViewCreateInfo const& createInfo) noexcept
        : image{image},
        subresource{createInfo.subresourceRange}
    { 
        EWE_VK(vkCreateImageView, image.logicalDevice.device, &createInfo, nullptr, &view);
    }
    
    ImageView::ImageView(Image& image) noexcept
        : image{image}
    {
        const auto createInfo = GetDefaultFullImageViewCreateInfo(image);
        subresource = createInfo.subresourceRange;

        EWE_VK(vkCreateImageView, image.logicalDevice.device, &createInfo, nullptr, &view);
    }

    ImageView::~ImageView(){
        vkDestroyImageView(image.logicalDevice.device, view, nullptr);
    }
}