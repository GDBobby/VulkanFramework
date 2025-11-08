#pragma once
#include "EightWinds/VulkanHeader.h"

#include <vector>
#include <string>
#include <unordered_map>

namespace EWE{

    //https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#vkCreateInstance
    //https://registry.khronos.org/vulkan/specs/latest/man/html/VkInstanceCreateInfo.html
    struct Instance{
        [[nodiscard]] Instance(const uint32_t api_version, std::vector<const char*> const& requiredExtensions, std::unordered_map<std::string, bool>& optionalExtensions, VkAllocationCallbacks const* allocCallbacks);

        VkInstance instance;

        operator VkInstance() const {return instance;}

        bool operator==(Instance const& other) const {
            return instance == other.instance;
        }
    };
}