#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/Backend/DescriptorPool.h"

#include "EightWinds/Data/KeyValueContainer.h"

#include <vector>

namespace EWE{


    namespace Descriptor{
        
        struct Set{
            //union{
            //    struct{
                    uint8_t index;
                    Bindings bindings;
            //    };
            //    KeyValuePair<uint8_t, Bindings> kvPair;
            //};
        };

        //this is for built descriptors, and designed to be binded to the pipeline/commandbuffer
        struct SetLayout{
            DescriptorPool& pool;

            [[nodiscard]] explicit SetLayout(DescriptorPool& pool) noexcept;
            SetLayout(SetLayout&& moveSrc) noexcept;
            SetLayout& operator=(SetLayout&& moveSrc) noexcept;

            SetLayout(const SetLayout&) = delete;
            SetLayout& operator=(const SetLayout&) = delete;

            Bindings bindings;
            const bool bindless; 
            VkDescriptorSetLayout vkDSL;

            
        private:
            void BuildVkDSL() noexcept;
        };

        //descriptorlayoutpack goes with a shader
        //it mirrors the set/binding layout that glsl uses
        //this kind of structure isn't too popular
        //in bindless strategies, swapping descriptor sets is defined by changing an index in the push constant
        //this isnt for built descriptors, just for reflection purposes
        struct LayoutPack{
            //cross reference the pools in each descriptor set layout potentially
            LayoutPack() = default;
            LayoutPack(LayoutPack&&) = default;
            LayoutPack& operator=(LayoutPack&&) = default;
            ~LayoutPack() = default;

            LayoutPack(LayoutPack const&) = delete;
            LayoutPack& operator=(LayoutPack const&) = delete;

            //what even is this
            //[[nodiscard]] VkDescriptorSetLayout* GetDescriptorSetLayout(uint8_t index);

            std::vector<Descriptor::Set> sets{}; //potentially sort by set index, idk if it matters
        };
    
    }//Descriptor
}