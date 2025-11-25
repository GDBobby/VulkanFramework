#pragma once

#include "EightWinds/VulkanHeader.h"

#include <vector>

namespace EWE{
    namespace Backend{
        namespace Descriptor{
            struct Bindings{

                std::vector<VkDescriptorSetLayoutBinding> vkBindings;
                std::vector<bool> writes;

                void Sort() noexcept;
            };
        } //namespace Descriptor
    } //namespace Backend
} //namespace EWE