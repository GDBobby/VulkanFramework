#pragma once
#include "EightWinds/VulkanHeader.h"
#include "EightWinds/Backend/DebugMessenger.h"

#include <vector>
#include <string>
#include <unordered_map>

namespace EWE{

    //https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#vkCreateInstance
    //https://registry.khronos.org/vulkan/specs/latest/man/html/VkInstanceCreateInfo.html
    struct Instance{
        [[nodiscard]] Instance(const uint32_t api_version, std::vector<const char*> const& requiredExtensions, std::unordered_map<std::string, bool>& optionalExtensions);

        VkInstance instance;
        const uint32_t api_version;

#if EWE_DEBUG_BOOL
        DebugMessenger debugMessenger;
#endif

        operator VkInstance() const {return instance;}

        bool operator==(Instance const& other) const {
            return instance == other.instance;
        }
    };
}