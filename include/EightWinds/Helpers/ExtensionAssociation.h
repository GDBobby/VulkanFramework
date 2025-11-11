#pragma once

#include "EightWinds/VulkanHeader.h"

#include <string_view>
#include <span>

namespace EWE{
    //VUID-VkDeviceCreateInfo-pNext-04748
    //If the pNext chain includes a VkPhysicalDeviceVulkan12Features structure and VkPhysicalDeviceVulkan12Features::bufferDeviceAddress is VK_TRUE, 
    //ppEnabledExtensionNames must not contain VK_EXT_buffer_device_address
    //^probably just disable that extension, or heavily discourage it somehow

    
    constexpr size_t MAX_DEPENDENCIES = 32;
    struct ExtensionAssociation {
        const char* name;
        VkStructureType feature_stype;
        VkStructureType property_stype;

        uint32_t promotion_version;    
        std::array<const char*, MAX_DEPENDENCIES> dependencies{};
        size_t dep_count = 0;

        [[nodiscard]] constexpr ExtensionAssociation(
            const char* name, 
            VkStructureType feature_stype, 
            VkStructureType property_stype, 
            std::span<const char*> deps = {}, 
            uint32_t promotion_version = UINT32_MAX
        ) 
            : name{name}, 
            feature_stype{feature_stype}, 
            property_stype{property_stype}, 
            promotion_version{promotion_version},
            dep_count{deps.size()}
        {
            //assert deps.size() < MAX_DEPENDNCIES
            for (size_t i = 0; i < dep_count; ++i) {
                dependencies[i] = deps[i];
            }
        }

        constexpr bool operator==(std::string_view other) const noexcept{
            return std::string_view(name) == other;
        }
        constexpr bool operator==(ExtensionAssociation const& other) const noexcept{
            return std::string_view(name) == std::string_view(other.name);
        }
    };

    extern constexpr std::array<const ExtensionAssociation> static_extension_associations;

    
    constexpr bool check_unique() {
        for (size_t i = 0; i < extension_associations.size(); ++i)
            for (size_t j = i + 1; j < static_extension_associations.size(); ++j)
                if (static_extension_associations[i] == static_extension_associations[j])
                    return false;
        return true;
    }
    static_assert(check_unique(), "Duplicate extension names in static_extension_associations!");


    template <ExtensionAssociation const* Ext>
    struct ExtensionIndex {
        static constexpr size_t value = []{
            // Suppose allExtensions is a constexpr array of all ExtensionAssociation
            for (size_t i = 0; i < allExtensions.size(); ++i) {
                if (&allExtensions[i] == Ext) return i;
            }
            return static_cast<size_t>(-1); // not found
        }();
    };
}