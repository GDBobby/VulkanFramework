#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/Helpers/ExtensionAssociation.h"

//one of the main reasons im avoiding hpp in the rest of the app is to reduce compile time. 
//i dont think it's avoidable here without retyping a very large portion of it
//just using structs.hpp would be a nice reduction in compile time but i believe that file was designed to have vulkan.hpp included first???
#include "vulkan/vulkan.hpp"

#include <vector>
#include <concepts>

namespace EWE{



    //0 extensions is default initialization
    struct DeviceExtension {

        const char* name;
        bool required;
        uint64_t score = 0;

        consteval [[nodiscard]] explicit DeviceExtension(const char* name, bool required, uint64_t score) noexcept
            : name{name}, required{required}, score{score}, supported{false}
        }

        bool supported = true; //this will be read after construction

        bool FailedRequirements() const {
            return required && !supported;
        }

        bool CheckSupport(std::vector<VkExtensionProperties> const& availableExtensions);
    };
    template<std::size_t N>
    constexpr auto extract_names(const std::array<DeviceExtension, N>& exts) {
        std::array<const char*, N> names{};
        for (std::size_t i = 0; i < N; ++i) {
            names[i] = exts[i].name;
        }
        return names;
    }

    //in C++26, we can use reflection to see WHICH members are failing, instead of just returning their address
    struct FailedRequirement{
        uint16_t indexInPNextChain;
        //every member has a uniform type of VkBool32
        //using that information, ill return an offset
        //so that it can be checked WHICH member it is. 
        //sType and pNext are not considered in the offset, so the first VkBool32 would be offset==0
        uint16_t offsetWithinStruct; 

        //when reflection is in, ill have this return a std::string_view with the struct and member's name for debugging
    };

    namespace Deduplicate{
        template <typename...>
        struct unique_types;

        template <>
        struct unique_types<> { using type = std::tuple<>; };

        template <typename T, typename... Ts>
        struct unique_types<T, Ts...> {
        private:
            using rest = typename unique_types<Ts...>::type;
        public:
            using type = std::conditional_t<
                (std::disjunction_v<std::is_same<T, Ts>...>),
                rest, // skip if duplicate
                decltype(std::tuple_cat(std::tuple<T>{}, rest))
            >;
        };

        template <typename... Ts>
        using unique_types_t = typename unique_types<Ts...>::type;
    }

    template <typename... Structs>
    struct VulkanStructContainer {
        std::tuple<SupportWrapper<Structs...>> data;

        consteval VulkanStructContainer() = default;

        template <typename T>
        T& Get() {
            return data;
        }
        template<typename T>
        bool IsSupported() const {
            return std::get<SupportWrapper<T>>(data).supported;
        }

        template<typename T>
        void SetSupported(bool value) {
            std::get<SupportWrapper<T>>(data).supported = value;
        }


        template <typename Func>
        requires (std::invocable<Func, Ts&> && ...)
        void ForEach(Func&& f) {
            ForEachImpl(std::forward<Func>(f), std::make_index_sequence<sizeof...(Ts)>{});
        }
        void* BuildPNextChain() {
            return BuildPNextChainImpl(std::make_index_sequence<sizeof...(Structs)>{});
        }

        template <typename... OtherStructs>
        requires
        consteval auto Append(const VulkanStructContainer<OtherStructs...>& other) const {
            auto combined = std::tuple_cat(data, other.data);
            using UniqueTuple = Deduplicate::unique_types_t<Structs..., OtherStructs...>;
            return VulkanStructContainerFromTuple<UniqueTuple>(combined);
        }
    private:
        template <typename Func, std::size_t... Is>
        void ForEachImpl(Func&& f, std::index_sequence<Is...>) {
            ( (f(std::get<Is>(data))), ... );
        }
        template <std::size_t... Is>
        void* BuildPNextChainImpl(std::index_sequence<Is...>) {
            if constexpr (sizeof...(Is) == 0) return nullptr;

            ( (SetPNextForIndex<Is, Is+1>()), ... );

            SetPNextForIndex<sizeof...(Is)-1, -1>();

            return &std::get<0>(data).underlyingType;
        }

        template <std::size_t I, int J>
        void SetPNextForIndex() {
            if constexpr (J == -1) {
                std::get<I>(data).underlyingType.pNext = nullptr;
            } else {
                std::get<I>(data).underlyingType.pNext = static_cast<void*>(&std::get<J>(data.underlyingType));
            }
        }
    };
            
    template <std::size_t ExtCount, std::size_t FeatureCount, std::size_t PropertyCount>
    struct ExtensionExpansionHelper {
        std::array<DeviceExtension, ExtCount>  extensions{};
        std::array<VkStructureType, FeatureCount> feature_types{};
        std::array<VkStructureType, PropertyCount> property_types{};

        consteval ExtensionExpansionHelper() = default;

        consteval ExtensionExpansionHelper(
            std::array<DeviceExtension, ExtCount> exts,
            std::array<VkStructureType, FeatureCount> feats,
            std::array<VkStructureType, PropertyCount> props
        )
        : extensions(exts), feature_types(feats), property_types(props) {}
    };

    template <std::size_t MaxExt = 256, //if my parser of the vk.xml is working these numbers cover over half of the total
            std::size_t MaxFeat = 128,
            std::size_t MaxProp = 128>
    constexpr auto ExpandExtensions(
        uint32_t vk_api_version,
        const std::array<DeviceExtension, MaxExt>& requested,
        std::size_t requested_count)
    {
        std::array<DeviceExtension, MaxExt> result_extensions{};
        std::array<VkStructureType, MaxFeat> result_features{};
        std::array<VkStructureType, MaxProp> result_properties{};
        std::size_t ext_count = 0;
        std::size_t feat_count = 0;
        std::size_t prop_count = 0;

        auto find_association = [&](std::string_view name) -> const ExtensionAssociation* {
            for (auto const& assoc : static_extension_associations){
                if (assoc.name == name) return &assoc;
            }
            return nullptr;
        };

        auto find_existing = [&](std::string_view name) -> DeviceExtension* {
            for (std::size_t i = 0; i < ext_count; ++i){
                if (result_extensions[i].name == name) return &result_extensions[i];
            }
            return nullptr;
        };

        auto contains = [](auto const& arr, std::size_t count, auto val) {
            for (std::size_t i = 0; i < count; ++i){
                if (arr[i] == val) return true;
            }
            return false;
        };

        std::function<void(std::string_view, bool)> addExtension;
        addExtension = [&](std::string_view name, bool required) {
            if (auto* existing = find_existing(name)) {
                if (required && !existing->required) {
                    existing->required = true;
                }
                return;
            }

            const ExtensionAssociation* assoc = find_association(name);
            if (!assoc) return;

            if (assoc->promotion_version && assoc->promotion_version <= vk_api_version) return;

            if (ext_count < MaxExt) {
                result_extensions[ext_count++] = DeviceExtension{name.data(), required, 0};
            }
            else{
                //static_assert potentially
                //only thing that needs to change is the callsize of this function
            }

            if (assoc->feature_stype != VK_STRUCTURE_TYPE_MAX_ENUM &&
                !contains(result_features, feat_count, assoc->feature_stype) &&
                feat_count < MaxFeat
            ) {
                result_features[feat_count++] = assoc->feature_stype;
            }

            if (assoc->property_stype != VK_STRUCTURE_TYPE_MAX_ENUM &&
                !contains(result_properties, prop_count, assoc->property_stype) &&
                prop_count < MaxProp
            ) {
                result_properties[prop_count++] = assoc->property_stype;
            }

            for (std::size_t i = 0; i < assoc->dep_count; ++i) {
                addExtension(assoc->dependencies[i], required);
            }
        };

        for (std::size_t i = 0; i < requested_count; ++i) {
            addExtension(requested[i].name, requested[i].required);
        }

        std::array<DeviceExtension, ext_count> exts{};
        std::array<VkStructureType, feat_count> feats{};
        std::array<VkStructureType, prop_count> props{};

        for (std::size_t i = 0; i < ext_count; ++i) {
            exts[i] = result_extensions[i];
        }
        for (std::size_t i = 0; i < feat_count; ++i) {
            feats[i] = result_features[i];
        }
        for (std::size_t i = 0; i < prop_count; ++i) {
            props[i] = result_properties[i];
        }

        return ExtensionExpansionHelper(exts, feats, props);
    }


    template <typename Array, std::size_t... Is>
    struct tuple_from_array_impl;

    template <typename T, std::size_t N, std::size_t... Is>
    struct tuple_from_array_impl<std::array<T, N>, Is...> {
        template <auto... Vs>
        using helper = std::tuple<typename CppType<Vs>::Type...>;
    };

    template <auto& arr, std::size_t... Is>
    using tuple_from_array_impl_t = std::tuple<typename CppType<decltype(arr[Is])>::Type...>;

    template<std::size_t ext_count, typename FeatureParamPack, typename PropertyParamPack>
    struct DeviceConstructionHelper{
        std::array<DeviceExtension, ext_count> extensions;
        VulkanStructContainer<FeatureParamPack> features;
        VulkanStructContainer<PropertyParamPack> properties;

        VkPhysicalDeviceFeatures2 feature_base;
        VkPhysicalDeviceProperties2 property_base;

        void DisableSupport(const char* name) {
            //i need runtime linkage between the name and the feature/property
            static_extension_assocations
        }


        bool ExtensionSupported(const char* name) const {
            //i need to build a compile time name to index relationship, but i dont have the emotional strength right now
            for(auto const& ext : extensions){
                if(ext == name){
                    return ext.supported;
                }
            }

            return false;
        }

        void PopulateFeatureData() noexcept {
            feature_base.pNext = features.BuildPNextChain();
        }
        void PopulatePropertyData() noexcept {
            property_base.pNext = properties.BuildPNextChain();
        }

        template<typename T>
        auto std::optional<T>& GetFeature() {
            if constexpr(std::is_same_v<T, VkPhysicalDeviceFeatures2>){
                return feature_base;
            }
            else{
                return features.GetSupported<T>() ? features.Get<T>() : std::nullopt;
            }
        }

        template<typename T>
        auto std::optional<T>& GetProperty() {
            if constexpr(std::is_same_v<T, VkPhysicalDeviceProperties2>){
                return property_base;
            }
            else{
                return properties.GetSupported<T>() ? properties.Get<T>() : std::nullopt;
            }
        }

        static [[nodiscard]] LogicalDevice ConstructDevice(VkPhysicalDevice physicalDevice){
            //check extensions
            
            uint32_t propCount;
            vkEnumerateDeviceExtensionProperties(physicalDevice.device, nullptr, &propCount, nullptr);
            std::vector<VkExtensionProperties> extensionProperties{propCount};
            vkEnumerateDeviceExtensionProperties(physicalDevice.device, nullptr, &propCount, extensionProperties.data());
            
            std::vector<const char*> supported_extensions{};

            for(auto& ext : extensions){
                if(ext.CheckSupport(extensionProperties)){
                    supported_extensions.push_back(ext.name);
                }
                else{
                    DisableSupport(name);
                }
            }
        }

        static [[nodiscard]] VkPhysicalDevice ScoreDevices(std::vector<VkPhysicalDevice> physicalDevices) {

        }
    };






















    //benched for now



    struct FeatureManager{
        //any feature that was set to true is considered required
        //the head is required to be VkPhysicalDeviceFeatures2
        VkPhysicalDeviceFeatures2* incomingFeatures = nullptr;
        std::vector<std::size_t> sizes;
        std::size_t totalSize = 0;
        VkBaseOutStructure* tail = nullptr; //basein makes the pNext a const* which is a bit annoying
        uint64_t chainCount = 0;

        //for comparison, the incomingfeatures will be copied to a second buffer
        //if outfeatures is not equal to nullptr on deconstruction, it will be called with Free
        VkPhysicalDeviceFeatures2* outFeatures = nullptr;

        /*
        * when C++26 comes, I'll enhance this so the active features can be retrieved
        template<typename T>
        T const& GetFeatureStruct(){
            //step thru each feature, do reflection comparison to get the feature. throw an exception if it's not available
            for(auto& feat : *this){
                if(std::is_same_v<T, declype(feat)){
                    return *feat;
                }
            }
        }
        */

        void SetHead(VkPhysicalDeviceFeatures2* head) noexcept {
            incomingFeatures = head;
            tail = reinterpret_cast<VkBaseOutStructure*>(head);
        }

        template<VulkanStruct VkStr>
        void AddFeature(VkStr& feature){
            sizes.push_back(sizeof(VkStr));
            totalSize += sizeof(VkStr);
            //assert(tail != nullptr);
            tail->pNext = reinterpret_cast<VkBaseOutStructure*>(&feature);
            tail = tail->pNext;
        }

        [[nodiscard]] std::vector<FailedRequirement> CheckFeatures(VkPhysicalDevice physicalDevice);
    private:
        //compare is meant to be called within CheckFeatures
        std::vector<FailedRequirement> Compare();
    };

    struct PropertyManager{
        /*
        * i'd like this struct to own the lifetime of each property
        * there's no point right now, i don't think there's a sophisticated way to access the contained data
        * so until then, i'll just do a normal chain and let the user define their own explicit access
        
        */
       //the head is required to be VkPhysicalDeviceProperties2
        VkPhysicalDeviceProperties2 head{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2, .pNext = nullptr};
        std::size_t totalSize = 0;
        //VkBaseOutStructure* tail = reinterpret_cast<VkBaseOutStructure*>(&head); //basein makes the pNext a const* which is a bit annoying
        uint64_t chainCount = 0;

        template<VulkanStruct... VulkanTypes>
        void AddPropertyStructs(VkPhysicalDevice device, VulkanTypes&... properties) noexcept {
            if constexpr(sizeof...(VulkanTypes) > 0){
                auto ptrs = std::array{ reinterpret_cast<VkBaseOutStructure*>(&properties)... };
                head->pNext = ptrs[0];

                for (size_t i = 0; i < (ptrs.size() - 1); ++i) {
                    ptrs[i]->pNext = ptrs[i + 1];
                }

                ptrs.back()->pNext = nullptr;
                //tail = ptrs.back();
            }
            
            vkGetPhysicalDeviceProperties2(device, &head);
        }
    };

}