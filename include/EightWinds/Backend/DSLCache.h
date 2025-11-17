#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/DescriptorSetLayout.h"

namespace EWE{
    namespace Backend{
            //were ignoring the count
            //which could potentially become an issue
        struct BindingsEqual {
            bool operator()(Descriptor::Bindings const& lhs, Descriptor::Bindings const& rhs) const {
                if (lhs.size() != rhs.size()) {
                    return false;
                }
                for (size_t i = 0; i < lhs.size(); ++i) {
                    const bool binding = lhs[i].binding != rhs[i].binding;
                    const bool descType = lhs[i].descriptorType != rhs[i].descriptorType;
                    //const bool descCount = lhs[i].descriptorCount != rhs[i].descriptorCount;
                    if(!binding || !descType){
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
        
        uint64_t BindingHash(const Descriptor::Bindings& bindings) {
            uint64_t hash = 0;
            size_t shift = 0;

            for (auto const& bind : bindings) {

                const uint64_t index = bind.binding & 0xF;
                // Pack 4 bits for binding index + 3 bits for descriptor type
                hash |= (index << shift);
                shift += 4;

                //assuming ALWAYS less than 15 bindings. might need a warning/assert for that
                const uint64_t code = DescriptorTypeCode(bind.descriptorType);
                hash |= (code << shift);
                shift += 3;
            }

            // MurmurHash3-style finalizer for bit mixing
            hash ^= hash >> 33;
            hash *= 0xff51afd7ed558ccdULL;
            hash ^= hash >> 33;
            hash *= 0xc4ceb9fe1a85ec53ULL;
            hash ^= hash >> 33;
            return hash;
        }
        
        struct DescriptorSetLayoutKeyHash {
            size_t operator()(const DescriptorSetLayoutKey& key) const {
                return BindingHash(key.bindings);
            }
        };


        struct DSLCache{


            LogicalDevice& logicalDevice;
            std::unordered_map<uint64_t, VkDescriptorSetLayout>
            [[nodiscard]] explicit DSLCache(LogicalDevice& logicalDevice) noexcept;
            ~DSLCache();
            VkDescriptorSetLayout Get(Descriptor::Bindings const& bindings) noexcept;
            void Free(VkDescriptorSetLayout) noexcept;

            std::unordered_map<Descriptor::Bindings, VkDescriptorSetLayout, DescriptorSetLayoutKeyHash, BindingsEqual> cache;

        };
    }
}