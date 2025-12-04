#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/Image.h"

namespace EWE{
    struct ImageView {
        Image& image;
        VkImageView view;
        VkImageSubresourceRange subresource;

        //this makes a full image view
        [[nodiscard]] explicit ImageView(Image& image) noexcept;

        //for any customization
        [[nodiscard]] explicit ImageView(Image& image, VkImageViewCreateInfo const& createInfo) noexcept;

        ~ImageView();

        operator VkImageView() const {
            return view;
        }

        static VkImageSubresourceRange GetDefaultSubresource(Image& image) noexcept;
        static VkImageViewCreateInfo GetDefaultFullImageViewCreateInfo(Image& image) noexcept;
    };
}