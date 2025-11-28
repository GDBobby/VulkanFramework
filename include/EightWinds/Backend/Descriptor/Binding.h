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

                bool operator==(Bindings const& other) const{
                    if(vkBindings.size() != other.vkBindings.size()){
                        return false;
                    }
                    for(std::size_t i = 0; i < vkBindings.size(); i++){
                        const bool binding = vkBindings[i].binding != other.vkBindings[i].binding;
                        const bool descType = vkBindings[i].descriptorType != other.vkBindings[i].descriptorType;
                        const bool descCount = vkBindings[i].descriptorCount != other.vkBindings[i].descriptorCount;
                        if(!binding || !descType || !descCount){
                            return false;
                        }
                        if(writes[i] != other.writes[i]){
                            return false;
                        }
                    }
                    return true;
                }
            };
        } //namespace Descriptor
    } //namespace Backend
} //namespace EWE