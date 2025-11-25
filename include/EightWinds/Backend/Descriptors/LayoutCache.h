#pragma once

#include "EightWinds/VulkanHeader.h"

#include "EightWinds/Backend/Descriptors/Binding.h"

#include <unordered_map>

namespace EWE{
    namespace Backend{
        namespace Descriptor{
            //the count isnt hashed
            //i think that means it gets put in the same bucket?
            struct BindingsEqual {
                bool operator()(Bindings const& lhs, Bindings const& rhs) const {
                    if (lhs.vkBindings.size() != rhs.vkBindings.size()) {
                        return false;
                    }
                    for (size_t i = 0; i < lhs.vkBindings.size(); ++i) {
                        const bool binding = lhs.vkBindings[i].binding != rhs.vkBindings[i].binding;
                        const bool descType = lhs.vkBindings[i].descriptorType != rhs.vkBindings[i].descriptorType;
                        const bool descCount = lhs.vkBindings[i].descriptorCount != rhs.vkBindings[i].descriptorCount;
                        if(!binding || !descType || !descCount){
                            return false;
                        }
                    }
                    return true;
                }
            };

            
            constexpr uint8_t DescriptorTypeCode(VkDescriptorType type) {
                switch(type){
                    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER: return 0;
                    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER: return 1;
                    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: return 2;
                    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE: return 3;
                    //others considered exceptionally rare, otherwise, there's room for 3 more here without expanding the required bits
                    default: return 4; // rare others
                }
            }
            
            struct BindingsHash{
                std::size_t operator()(Bindings const& bindings) const {
                    uint64_t hash = 0;
                    size_t shift = 0;

                    for (auto const& bind : bindings.vkBindings) {

                        const uint64_t index = bind.binding & 0xF;
                        // Pack 4 bits for binding index + 3 bits for descriptor type
                        hash |= (index << shift);
                        shift += 4;

                        //assuming ALWAYS less than 15 bindings. might need a warning/assert for that
                        const uint64_t code = DescriptorTypeCode(bind.descriptorType);
                        hash |= (code << shift);
                        shift += 3;
                    }

                    // MurmurHash3
                    hash ^= hash >> 33;
                    hash *= uint64_t(0xff51afd7ed558ccd);
                    hash ^= hash >> 33;
                    hash *= uint64_t(0xc4ceb9fe1a85ec53);
                    hash ^= hash >> 33;
                    return hash;
                }
            };


            struct LayoutCache{
                VkDevice device;

                [[nodiscard]] explicit LayoutCache(VkDevice device) noexcept;
                ~LayoutCache();

                [[nodiscard]] VkDescriptorSetLayout Get(Descriptor::Bindings const& bindings) noexcept;
                void Free(VkDescriptorSetLayout) noexcept;

                //https://martin.ankerl.com/2022/08/27/hashmap-bench-01/
                //if the hashing speed becomes an issue, check that^
                //this is designed to be interchangeable behind the scenes, so that it can be improved/changed/whatever without reworking an engine
            private:
                std::unordered_map<Bindings, VkDescriptorSetLayout, BindingsHash, BindingsEqual> cache;

            };
        } //namespace Descriptor
    } //namespace Backend
} //namespace EWE