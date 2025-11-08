#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/DescriptorPool.h"

namespace EWE{

    //descriptorsetlayouts may be over
    struct DescriptorSetLayout{
        DescriptorPool& pool;

        DescriptorSetLayout(DescriptorPool& pool) : pool{pool} {}
        //std::vector<DescriptorSet> bindings{}; //possible to skip a binding, need a key value pair
    };
}