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
    struct RequestedExtension {

        std::size_t name_index;
        bool required;
        uint64_t score = 0;

        consteval [[nodiscard]] explicit DeviceExtension(const char* name, bool required, uint64_t score) noexcept
            : name{name}, required{required}, score{score}, supported{false}
        }

        //this will be read after construction
        //set in the CheckSupport function
        bool supported = false; 

        bool FailedRequirements() const {
            return required && !supported;
        }

        bool CheckSupport(std::vector<VkExtensionProperties> const& availableExtensions) {
            if(supported){
                return true;
            }
            for(auto& avail : availableExtensions){
                if(static_extension_associations[name_index] == avail.name){
                    supported = true;
                    return true;
                }
            }
        }
    };


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

    constexpr uint32_t RoundDownVkVersion(uint32_t in_version) noexcept{
        constexpr uint32_t mask = (1 << 12) - 1;
        constexpr uint32_t inverted_mask = ~mask;
        return in_version & inverted_mask;
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
        std::tuple<Structs...> data;
        static constexpr bool HasDuplicates = !std::is_same_v<std::tuple<Structs...>, Deduplicate::unique_types_t<Structs...>>;

        consteval VulkanStructContainer() = default;

        template <typename T>
        requires ((std::same_as<T, Structs> || ...))
        T& Get() {
            return data;
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

            return &std::get<0>(data);
        }

        template <std::size_t I, int J>
        void SetPNextForIndex() {
            if constexpr (J == -1) {
                std::get<I>(data).pNext = nullptr;
            } else {
                std::get<I>(data).pNext = static_cast<void*>(&std::get<J>(data));
            }
        }
    };
            
    template<uint32_t Vk_Version, typename FeatureParamPack, typename PropertyParamPack>
    struct DeviceConstructionHelper{
        VkPhysicalDeviceProperties2 property_base;
        //v1.1 includes features11, v1.2 for features12 and so on
        using VersionPropertyPack = typename VulkanVersionTypes<RoundDownVkVersion(Vk_Version)>::PropertyTypes;
        using FinalPropertyPack = decltype(Deduplicate::unique_types_t(std::tuple<PropertyParamPack...>{}, VersionPropertyPack{}));
        VulkanStructContainer<FinalPropertyPack> properties;

        std::vector<DeviceExtension> extensions;

        template<typename T>
        auto T& GetFeature()        
        requires (std::is_same_v<T, VkPhysicalDeviceFeatures2> || (std::same_as<T, FinalFeaturePack> || ...)
        ) {
            if constexpr(std::is_same_v<T, VkPhysicalDeviceFeatures2>){
                return feature_base;
            }
            else{
                return features.Get<T>();
            }
        }
        template<typename T>
        const T& GetFeature() const
        requires (std::is_same_v<T, VkPhysicalDeviceFeatures2> || (std::same_as<T, FinalFeaturePack> || ...))
        {
            if constexpr (std::is_same_v<T, VkPhysicalDeviceFeatures2>) {
                return feature_base;
            } else {
                return features.Get<T>();
            }
        }

        template<typename T>
        auto T& GetProperty()
        requires (std::is_same_v<T, VkPhysicalDeviceProperties2> || (std::same_as<T, FinalPropertyPack> || ...))
        {   
            if constexpr(std::is_same_v<T, VkPhysicalDeviceProperties2>){
                return property_base;
            }
            else{
                return properties.Get<T>();
            }   
        }
        template<typename T>
        const T& GetProperty() const
            requires (std::is_same_v<T, VkPhysicalDeviceProperties2> || (std::same_as<T, FinalPropertyPack> || ...))
        {
            if constexpr (std::is_same_v<T, VkPhysicalDeviceProperties2>) {
                return property_base;
            } else {
                return properties.Get<T>();
            }
        }

        bool ExtensionSupported(const char* name) const {
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


        static [[nodiscard]] LogicalDevice ConstructDevice(VkPhysicalDevice physicalDevice){
            //check extensions
            
            uint32_t propCount;
            vkEnumerateDeviceExtensionProperties(physicalDevice.device, nullptr, &propCount, nullptr);
            std::vector<VkExtensionProperties> extensionProperties{propCount};
            vkEnumerateDeviceExtensionProperties(physicalDevice.device, nullptr, &propCount, extensionProperties.data());
            
            std::vector<const char*> supported_extensions{};

            uint64_t score = 0;
            for(auto& ext : extensions){
                if(ext.CheckSupport(extensionProperties)){
                    supported_extensions.push_back(ext.name);
                    score += supported_extensions.score;
                }
                else{
                    DisableSupport(name);
                }
            }
        }

        static [[nodiscard]] VkPhysicalDevice ScoreDevices(std::vector<VkPhysicalDevice> physicalDevices) {

        }
    };




















    //i'd like to merge properties and features into one template base but it might not be worth the effort
    template<uint32_t Vk_Version, typename FeatureParamPack>
    struct FeatureManager{
        
        using VersionFeaturePack = typename VulkanVersionTypes<RoundDownVkVersion(Vk_Version)>::FeatureTypes;
        using FinalFeaturePack = decltype(Deduplicate::unique_types_t(std::tuple<FeatureParamPack...>{}, VersionFeaturePack{}));
        VulkanStructContainer<FinalFeaturePack> features;
        VkPhysicalDeviceFeatures2 base;

        static FeatureManager PopulateFeatures(VkPhysicalDevice physicalDevice){
            FeatureManager featureManager{};
            //need to populate sTypes
            //if i use sTypes to populate the templates for this struct that could make it a bit easier
            featureManager.base.stype = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
            featureManager.features.ForEach([](){
                //fix this, it's not complete
                data.sType = vk::CppType<feature_type>::Type::SType::GetVkStype;
            };)

            base.pNext = features.BuildPNextChain();
            vkGetPhysicalDeviceFeatures2(physicalDevice, &base);

            auto copiedFeatures = 
        }

        uint64_t ScoreFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2 copied_chain) const{

        }

        uint64_t ScoreFeatures(VkPhysicalDevice physicalDevice) const {
            uint64_t ret;
        }
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