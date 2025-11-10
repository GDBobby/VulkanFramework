#pragma once

#include "EightWinds/VulkanHeader.h"

#include "vulkan/vulkan_structs.hpp" //this is used for some mild amount of reflection. tying sType to the actual VkStructure

#include <vector>
#include <concepts>

namespace EWE{
/*
    i need to tie associated features and properties to const char*
    i could do a template for DeviceExtension, and define the structs as types.
    dont need to define the data, just save the types.
    but type erasure will be needed, so that i can store the device extensions in a container. tuple maybe?

    or i could get a pointer to the device extension.
    thats my least favorite option, i'd still need to manually control the other structs, 
    as opposed to here where I can send it back.

    ill need to handle the stype regardless. stype is miserable af


    the associations between const char* extensions and features/properties shouldnt need to be created even by every programmer/application
    i'd almost go as far as vulkan shouldve added an optional utility to provide this association
    
    altho maybe not provided by vulkan directly since this would be a C++ thing (but still probably)
*/

    template<VkStructureType SType>
    constexpr vk::StructureType PromoteSTypeToCPP(){
        return static_cast<vk::StructureType>(SType);
    }
    
    template<VkStructureType ST, typename = void>
    struct has_cpp_type : std::false_type {};

    template<VkStructureType ST>
    struct has_cpp_type<
        ST,
        std::void_t<typename vk::CppType<vk::StructureType, static_cast<vk::StructureType>(ST)>::Type>
    > : std::true_type {};


    template<vk::StructureType S>
    inline constexpr bool has_cpp_type_v = has_cpp_type<S>::value;

    template<const char* extension_name>
    struct ExtensionAssociation : {
        static_assert(false == 0, "No specialization of ExtensionAssociation<> for this extension.");
    };

    template<>
    struct ExtensionAssociation<VK_KHR_SWAPCHAIN_EXTENSION_NAME>{
        static constexpr VkStructureType featureSType = VK_STRUCTURE_TYPE_MAX_ENUM;
        static constexpr VkStructureType propertySType = VK_STRUCTURE_TYPE_MAX_ENUM;
        static constexpr VkStructureType pNextSType = VK_STRUCTURE_TYPE_MAX_ENUM;
    };

    template<>
    struct ExtensionAssociation<VK_EXT_descriptor_indexing_name>{
        static constexpr VkStructureType featureSType  = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
        static constexpr VkStructureType propertySType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES;
        static constexpr VkStructureType pNextSType    = VK_STRUCTURE_TYPE_MAX_ENUM;
    };



    template<const char* Name>
    requires{
        { ExtensionAssociation<Name>::featureSType }  -> std::convertible_to<VkStructureType>;
        { ExtensionAssociation<Name>::propertySType } -> std::convertible_to<VkStructureType>;
        { ExtensionAssociation<Name>::pNextSType }    -> std::convertible_to<VkStructureType>;
    }
    struct ExtensionAssociationImpl {
        static constexpr const char* name = Name;
        using Derived = ExtensionAssociation<Name>;
    private:
        static constexpr VkStructureType FeatureSType  = Derived::featureSType;
        static constexpr VkStructureType PropertySType = Derived::propertySType;
        static constexpr VkStructureType PNextSType    = Derived::pNextSType;

    public:

        static constexpr bool has_feature  = FeatureSType  != VK_STRUCTURE_TYPE_MAX_ENUM && has_cpp_type_v<FeatureSType>;
        static constexpr bool has_property = PropertySType != VK_STRUCTURE_TYPE_MAX_ENUM && has_cpp_type_v<PropertySType>;
        static constexpr bool has_pnext    =  PNextSType    != VK_STRUCTURE_TYPE_MAX_ENUM && has_cpp_type_v<PNextSType>;

        using Feature  = std::conditional_t<has_feature,  CppTypeFromSType_t<FeatureSType>, void>;
        using Property = std::conditional_t<has_property, CppTypeFromSType_t<PropertySType>, void>;
        using PNext    = std::conditional_t<has_pnext,    CppTypeFromSType_t<PNextSType>, void>;

        static constexpr Feature    GetFeature()    requires(!std::is_void_v<Feature>)  { return Feature{}; }
        static constexpr Property   GetProperty()   requires(!std::is_void_v<Property>) { return Property{}; }
        static constexpr PNext      GetPNext()      requires(!std::is_void_v<PNext>)    { return PNext{}; }
    };

    //0 extensions is default initialization
    struct DeviceExtension {

        const char* name;
        bool required;
        uint64_t score = 0;

        [[nodiscard]] explicit DeviceExtension(VkBaseOutStructure* pNextChain, const char* name, bool required, uint64_t score) noexcept
            : name{name}, required{required}, score{score},
            //if it doesnt have a name, it should always be available. this is thigns like devicefeatures2.
            supported{name == nullptr}
        {
            //the score should be 0 if the name is nullptr, since it's always available. but whatever, i'll leave it
        }

        bool supported; //this will be read after construction

        bool FailedRequirements() const {
            return required && !supported;
        }

        bool CheckSupport(std::vector<VkExtensionProperties> const& availableExtensions);
    };


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