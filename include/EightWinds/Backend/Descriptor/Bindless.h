#pragma once

#include "EightWinds/VulkanHeader.h"

#include <bitset>
#include <array>

namespace EWE{
    struct LogicalDevice;
    enum class DescriptorType {
        Sampler,
        Combined,
        Sampled,
        Storage,

        COUNT
    };
    constexpr DescriptorType VkCompactDescriptorType(VkDescriptorType type) {
        switch (type) {
            case VK_DESCRIPTOR_TYPE_SAMPLER: return DescriptorType::Sampler;
            case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: return DescriptorType::Combined;
            case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE: return DescriptorType::Sampled;
            case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE: return DescriptorType::Storage;
        }

        EWE_UNREACHABLE;
    }
    constexpr VkDescriptorType VkExpandDescriptorType(DescriptorType type) {
        switch (type) {
            case DescriptorType::Sampler: return VK_DESCRIPTOR_TYPE_SAMPLER;
            case DescriptorType::Combined: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            case DescriptorType::Sampled: return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            case DescriptorType::Storage: return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        }
        EWE_UNREACHABLE;
        return VK_DESCRIPTOR_TYPE_SAMPLER; //error silencer
    }

    namespace Backend{

        //images only. use DBA for buffers
        struct BindlessDescriptor{
            LogicalDevice& logicalDevice;

            [[nodiscard]] explicit BindlessDescriptor(LogicalDevice& logicalDevice);
            ~BindlessDescriptor();


            static constexpr std::size_t max_images_per_type{UINT16_MAX};

            std::array<std::bitset<max_images_per_type>, static_cast<size_t>(DescriptorType::COUNT)> occupancy;

            DescriptorIndex BindImage(VkDescriptorImageInfo const& imageInfo, DescriptorType descriptorType);

            void Unbind(DescriptorIndex index, DescriptorType type);

            //i can overfill this with information a bit since it's only every going to be created once
            VkDescriptorPool pool;
            VkDescriptorSetLayout layout;

            VkDescriptorSet set;

            /*
            i think ideally, i'd have information like
            struct BindingData{
                VkDescriptorSetLayoutBinding,
                VkDescriptorBindingFlags,
                pool size,

                whatever other data is tied to binding count in here
            }
                std::vector<Bindingdata> bindingData;

                //but idk if its worth the trouble (not a lot of trouble but still)
            */
            std::array<VkDescriptorSetLayoutBinding, static_cast<size_t>(DescriptorType::COUNT)> bindings;

        private:
            //id prefer to put these in the implementation only, but I need to set other members. by reference maybe?
            void CreateLayout();
            void CreatePool();
            void CreateDescriptor();
        };
    } //namespace Backend
} //namespace EWE