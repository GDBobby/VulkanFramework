#include "EightWinds/DescriptorSetLayout.h"

#include <cassert>

#include <stdexcept>

namespace EWE{
    namespace Descriptor{

        void SetLayout::BuildVkDSL() noexcept {
            VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
            descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
            descriptorSetLayoutInfo.pBindings = bindings.data();

            if (bindless) {
                const VkDescriptorBindingFlagsEXT flags =
                    VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT |
                    VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT |
                    VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT |
                    VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT_EXT;

                VkDescriptorSetLayoutBindingFlagsCreateInfoEXT binding_flags{};
                binding_flags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
                binding_flags.pNext = nullptr;
                binding_flags.bindingCount = 1;
                binding_flags.pBindingFlags = &flags;
                descriptorSetLayoutInfo.pNext = &binding_flags;

                descriptorSetLayoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;

                EWE_VK(vkCreateDescriptorSetLayout, pool.logicalDevice.device, &descriptorSetLayoutInfo, nullptr, &vkDSL);
            }
            else {
                descriptorSetLayoutInfo.flags = 0;
                descriptorSetLayoutInfo.pNext = nullptr;
                EWE_VK(vkCreateDescriptorSetLayout, pool.logicalDevice.device, &descriptorSetLayoutInfo, nullptr, &vkDSL);
            }
        }

        SetLayout::SetLayout(DescriptorPool& pool) noexcept 
            : pool{ pool }, bindless{ false } 
        {
            
        }

        SetLayout::SetLayout(SetLayout&& moveSrc) noexcept
            : pool{moveSrc.pool}, bindless{moveSrc.bindless}

        {
            vkDSL = moveSrc.vkDSL;
            moveSrc.vkDSL = VK_NULL_HANDLE;
        }
        SetLayout& SetLayout::operator=(SetLayout&& moveSrc) noexcept{
            assert(pool == moveSrc.pool);

            throw std::runtime_error("not fulyl setup, the old dsl needs to be deconstructed");
            vkDSL = moveSrc.vkDSL;
            return *this;
        }
    } //namespace Descriptor
} //namespace EWE