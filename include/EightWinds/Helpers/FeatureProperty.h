#pragma once


#include "EightWinds/VulkanHeader.h"

#include "vulkan/vulkan.hpp"

#include <concepts>
#include <tuple>
#include <utility>
#include <type_traits>
#include <cstdint>

namespace EWE {
    template<VulkanStruct VkStr>
    void SetVkStructSType(VkStr& vkstr) {
        vkstr.sType = static_cast<VkStructureType>(vk::CppType<VkStr>::Type::structureType);
    }

    template<uint32_t VkVersion>
    struct VulkanVersionTypes {
        using FeatureTypes = std::tuple<>;
        using PropertyTypes = std::tuple<>;
    };

    template<>
    struct VulkanVersionTypes<VK_API_VERSION_1_1> {
        using FeatureTypes = std::tuple<VkPhysicalDeviceVulkan11Features>;
        using PropertyTypes = std::tuple<VkPhysicalDeviceVulkan11Properties>;
    };

    template<>
    struct VulkanVersionTypes<VK_API_VERSION_1_2> {
        using FeatureTypes = std::tuple<VkPhysicalDeviceVulkan11Features, VkPhysicalDeviceVulkan12Features>;
        using PropertyTypes = std::tuple<VkPhysicalDeviceVulkan11Properties, VkPhysicalDeviceVulkan12Properties>;
    };

    template<>
    struct VulkanVersionTypes<VK_API_VERSION_1_3> {
        using FeatureTypes = std::tuple<VkPhysicalDeviceVulkan11Features, VkPhysicalDeviceVulkan12Features, VkPhysicalDeviceVulkan13Features>;
        using PropertyTypes = std::tuple<VkPhysicalDeviceVulkan11Properties, VkPhysicalDeviceVulkan12Properties, VkPhysicalDeviceVulkan13Properties>;
    };

    template<>
    struct VulkanVersionTypes<VK_API_VERSION_1_4> {
        using FeatureTypes = std::tuple<VkPhysicalDeviceVulkan11Features, VkPhysicalDeviceVulkan12Features, VkPhysicalDeviceVulkan13Features, VkPhysicalDeviceVulkan14Features>;
        using PropertyTypes = std::tuple<VkPhysicalDeviceVulkan11Properties, VkPhysicalDeviceVulkan12Properties, VkPhysicalDeviceVulkan13Properties, VkPhysicalDeviceVulkan14Properties>;
    };

    namespace Deduplication {
        template <typename...>
        struct TypeList {};

        template <typename T, typename... Ts>
        struct Contains;

        template <typename T>
        struct Contains<T> : std::false_type {};

        template <typename T, typename Head, typename... Tail>
        struct Contains<T, Head, Tail...> : std::conditional_t<
            std::is_same_v<T, Head>,
            std::true_type,
            Contains<T, Tail...>
        >{};

        template <typename... Ts>
        struct HasDuplicates;

        template <>
        struct HasDuplicates<> : std::false_type {};

        template <typename Head, typename... Tail>
        struct HasDuplicates<Head, Tail...> : std::conditional_t<
            Contains<Head, Tail...>::value,
            std::true_type,
            HasDuplicates<Tail...>
        > {
        };

        template <typename Tuple>
        struct TupleToTypeList;

        template <typename... Ts>
        struct TupleToTypeList<std::tuple<Ts...>> {
            using type = TypeList<Ts...>;
        };

        template <typename TL, typename T>
        struct AppendIfNotExists;

        template <typename... Ts, typename T>
        struct AppendIfNotExists<TypeList<Ts...>, T> {
            using type = std::conditional_t<
                Contains<T, Ts...>::value,
                TypeList<Ts...>,
                TypeList<Ts..., T>
            >;
        };

        template <typename TL1, typename TL2>
        struct MergeNoDuplicates;

        template <typename... Ts>
        struct MergeNoDuplicates<TypeList<Ts...>, TypeList<>> {
            using type = TypeList<Ts...>;
        };

        template <typename... Ts, typename Head, typename... Tail>
        struct MergeNoDuplicates<TypeList<Ts...>, TypeList<Head, Tail...>> {
            using type = typename MergeNoDuplicates<
                typename AppendIfNotExists<TypeList<Ts...>, Head>::type,
                TypeList<Tail...>
            >::type;
        };

        // Convert TypeList back to tuple
        template <typename TL>
        struct TypeListToTuple;

        template <typename... Ts>
        struct TypeListToTuple<TypeList<Ts...>> {
            using type = std::tuple<Ts...>;
        };
    }


    template <typename... Structs>
    struct VulkanStructContainer {
        static_assert(!Deduplication::HasDuplicates<Structs...>::value, "Duplicate types in VulkanStructContainer!");

        template<typename T>
        static constexpr bool Contains_Type = (std::same_as<T, Structs> || ...);

        std::tuple<Structs...> data;

        constexpr VulkanStructContainer() = default;

        template <typename T>
        requires Contains_Type<T>
        [[nodiscard]] T& Get() {
            return std::get<T>(data);
        }

        template <typename T>
        requires Contains_Type<T>
        [[nodiscard]] T const& Get() const {
            return std::get<T>(data);
        }

        template <typename Func>
        requires (std::invocable<Func, Structs&> && ...)
        void ForEach(Func&& f) {
            ForEachImpl(std::forward<Func>(f), std::make_index_sequence<sizeof...(Structs)>{});
        }
        void* BuildPNextChain() {
            return BuildPNextChainImpl(std::make_index_sequence<sizeof...(Structs)>{});
        }

    private:
        template <typename Func, std::size_t... Is>
        void ForEachImpl(Func&& f, std::index_sequence<Is...>) {
            ((f(std::get<Is>(data))), ...);
        }
        template <std::size_t... Is>
        [[nodiscard]] void* BuildPNextChainImpl(std::index_sequence<Is...>) {
            if constexpr (sizeof...(Is) == 0) return nullptr;

            ((SetPNextForIndex<Is, Is + 1>()), ...);

            SetPNextForIndex<sizeof...(Is) - 1, -1>();

            return &std::get<0>(data);
        }

        template <std::size_t I, int J>
        void SetPNextForIndex() {
            SetVkStructSType(std::get<I>(data));
            if constexpr (J == -1) {
                std::get<I>(data).pNext = nullptr;
            }
            else {
                std::get<I>(data).pNext = static_cast<void*>(&std::get<J>(data));
            }
        }
    };

    template <typename Tuple>
    struct TupleToVulkanStructContainer;

    template <typename... Ts>
    struct TupleToVulkanStructContainer<std::tuple<Ts...>> {
        using type = VulkanStructContainer<Ts...>;
    };

    //i'd like to merge properties and features into one template base but it might not be worth the effort
    template<uint32_t Vk_Version, typename... FeatureParamPack>
    struct FeatureManager {

        VkPhysicalDeviceFeatures2 base;
        //v1.1 includes features11, v1.2 for features12 and so on
        using VersionFeaturePack = typename VulkanVersionTypes<RoundDownVkVersion(Vk_Version)>::FeatureTypes;
        using TupleFeaturePack = std::tuple<FeatureParamPack...>;
        using CombinedTypeList = typename Deduplication::MergeNoDuplicates<
            typename Deduplication::TupleToTypeList<VersionFeaturePack>::type,
            typename Deduplication::TupleToTypeList<TupleFeaturePack>::type
        >::type;
        using FinalFeaturePack = typename Deduplication::TypeListToTuple<CombinedTypeList>::type;
        using FeatureContainer = typename TupleToVulkanStructContainer<FinalFeaturePack>::type;

        FeatureContainer features;

        template<typename T>
        static constexpr bool Contains_Type = (std::same_as<T, VkPhysicalDeviceFeatures2> || FeatureContainer::template Contains_Type<T>);

        void Populate(VkPhysicalDevice physicalDevice) {
            base.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
            
            if (base.pNext == nullptr) {
                base.pNext = features.BuildPNextChain();
            }
            vkGetPhysicalDeviceFeatures2(physicalDevice, &base);
        }

        [[nodiscard]] uint64_t Score(VkPhysicalDevice physicalDevice) const {
            uint64_t ret = 0;
            features.ForEach([&ret](auto& feature) {
                    using FeatureType = std::decay_t<decltype(feature)>;
                    ret += DeviceScoring<FeatureType>{}(feature);
                }
            );

            return ret;
        }
    };

    template<uint32_t Vk_Version, typename... PropertyParamPack>
    struct PropertyManager {

        VkPhysicalDeviceProperties2 base;
        //v1.1 includes features11, v1.2 for features12 and so on
        using VersionPropertyPack = typename VulkanVersionTypes<RoundDownVkVersion(Vk_Version)>::PropertyTypes;
        using TuplePropertyPack = std::tuple<PropertyParamPack...>;
        using CombinedTypeList = typename Deduplication::MergeNoDuplicates<
            typename Deduplication::TupleToTypeList<VersionPropertyPack>::type,
            typename Deduplication::TupleToTypeList<TuplePropertyPack>::type
        >::type;
        using FinalPropertyPack = typename Deduplication::TypeListToTuple<CombinedTypeList>::type;
        using PropertyContainer = typename TupleToVulkanStructContainer<FinalPropertyPack>::type;

        PropertyContainer properties;

        template<typename T>
        static constexpr bool Contains_Type = (std::same_as<T, VkPhysicalDeviceProperties2> || PropertyContainer::template Contains_Type<T>);

        void Populate(VkPhysicalDevice physicalDevice) {
            base.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
            if (base.pNext == nullptr) {
                base.pNext = properties.BuildPNextChain();
            }

            vkGetPhysicalDeviceProperties2(physicalDevice, &base);
        }

        [[nodiscard]] uint64_t Score(VkPhysicalDevice physicalDevice) const {
            uint64_t ret = 0;
            properties.ForEach([&ret](auto& property) {
                using PropertyType = std::decay_t<decltype(property)>;
                ret += DeviceScoring<PropertyType>{}(property);
            }
            );

            return ret;
        }
    };

} //namespace EWE