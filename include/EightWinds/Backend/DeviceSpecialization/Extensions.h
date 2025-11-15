#pragma once

#include "EightWinds/VulkanHeader.h"

#include <concepts>
#include <cstdint>

namespace EWE {


    //in C++26, we can use reflection to see WHICH members are failing, instead of just returning their address
    struct FailedRequirement {
        uint16_t indexInPNextChain;
        //every member has a uniform type of VkBool32
        //using that information, ill return an offset
        //so that it can be checked WHICH member it is. 
        //sType and pNext are not considered in the offset, so the first VkBool32 would be offset==0
        uint16_t offsetWithinStruct;

        //when reflection is in, ill have this return a std::string_view with the struct and member's name for debugging
    };




    template <size_t N>
    struct ConstEvalStr {
        char value[N]{};
        consteval ConstEvalStr(const char(&str)[N]) {
            std::copy_n(str, N, value);
        }
        consteval operator std::string_view() const { return { value, N - 1 }; }
        constexpr bool operator==(ConstEvalStr const& other) const {
            for (size_t i = 0; i < N; ++i) {
                if (value[i] != other.value[i]) {
                    return false;
                }
            }
            return true;
        }
        consteval std::string_view view() {return std::string_view(value, N -1); }
    };

    template <auto Name, bool Req, uint64_t Score>
    struct ExtensionEntry {
        static constexpr auto name = Name;
        static constexpr bool required = Req;
        static constexpr uint64_t score = Score;
    };

    template<uint32_t Vk_Version, typename... exts>
    struct ExtensionManager {

        static consteval size_t Ext_Count() { return sizeof...(exts); }
        static constexpr std::array<std::string_view, Ext_Count()> names = { exts::name... };
        static constexpr std::array<uint64_t, Ext_Count()> scores = {exts::score...};
        static constexpr std::array<bool, Ext_Count()> required = {exts::required...};

        static consteval bool UniqueParamPack() {
            for (std::size_t i = 0; i < names.size(); ++i) {
                for (std::size_t j = i + 1; j < names.size(); ++j) {
                    if (names[i] == names[j]) {
                        return false;
                    }
                }
            }
            return true;
        }
        static_assert(UniqueParamPack(), "duplicate extension names in ExtensionManager");

        static constexpr std::size_t NameToIndex(std::string_view name) {
            for (std::size_t i = 0; i < names.size(); ++i) {
                if (names[i] == name) {
                    return i;
                }
            }
            //throw or static_assert
        }

        static std::vector<bool> GetSupport(std::vector<VkExtensionProperties> const& extProps) {
            std::vector<bool> supported = std::vector<bool>(names.size(), false);
            for (uint16_t i = 0; i < names.size(); i++) {
                for (auto const& extProp : extProps) {
                    if (names[i] == extProp.extensionName) {
                        supported[i] = true;
                        break;
                    }
                }
            }
            return supported;
        }
    };
}