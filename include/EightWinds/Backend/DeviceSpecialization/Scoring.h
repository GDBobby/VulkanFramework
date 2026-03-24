#pragma once

#include "EightWinds/VulkanHeader.h"

#include <vector>
#include <concepts>

namespace EWE{

    enum class WeightComparison{
        minimum,
        equal,
        maximum,
        //could do minimum equal, minimum not equal. not sure if necessary
        //other
    };

    template<typename T, WeightComparison comparison = WeightComparison::minimum>
    struct DeviceWeight{
        using WeightType = T;
        static constexpr auto comparison_v = comparison;
        //potentially add a minimum required spec instead of just a bool
        //i actually odnt know if multiple versions of a single extension/feature/limit have been added
        //and a programmer would accept one version but not another
        WeightType required;
        uint64_t score;

        constexpr bool CheckRequirement(WeightType val) const noexcept {
            //as far as i know, bigger is always better.
            //i BELIEVE i can ALWAYS use a minimum value
            //but i can use a templated comparison anyways
            if constexpr(comparison_v == WeightComparison::minimum){
                return val >= required;
            }
            else if constexpr(comparison_v == WeightComparison::maximum){
                return val <= required;
            }
            else if constexpr(comparison_v == WeightComparison::equal){
                return val == required;
            }
            //static_assert_false
        }
    

        constexpr DeviceWeight(WeightType _required, uint64_t _score) : required{_required}, score{_score} {}
    };

    //static consteval auto DW_GetMembers(){
    //    return std::span{
    //        std::define_static_array(std::meta::nonstatic_data_members_of(T, std::meta::access_context::current())).data(),
    //        MetaFunc(T, std::meta::access_context::current()).size()
    //    };
    //}

    template<class T>
    struct DefaultWeights{
        //static constexpr auto member_meta_info = DW_GetMembers();
        //static std::array<DeviceWeight<
    };

    template<>
    struct DefaultWeights<VkPhysicalDeviceFeatures>{
        static constexpr auto samplerAnisotropy = DeviceWeight<bool>(true, 1 << 8);
    };

    template<>
    struct DefaultWeights<VkPhysicalDeviceLimits>{
        static constexpr auto maxImageDimension1D = DeviceWeight<uint32_t>(256, 3); //each additional texel allowed in this dimensions adds the second parameter (3) to the score
        static constexpr auto maxImageDimension2D = DeviceWeight<uint32_t>(256, 3);
        static constexpr auto maxImageDimension3D = DeviceWeight<uint32_t>(256, 3);
    };

    template<>
    struct DefaultWeights<VkPhysicalDeviceProperties>{
        //i need to pass thru the limits somehow
        //static constexpr auto
    };

    struct DeviceScore{
        bool metRequirements = true;
        uint64_t score = 0;

        void Add(DeviceScore const& other) {
            //i forget if &= is bitwise or not
            metRequirements = metRequirements && other.metRequirements;
            score += other.score;
        }
    };

    template<typename T>
    struct DeviceScoring{
        using ScoredType = void;
        DeviceScore operator()(T const& src) const noexcept {
            return DeviceScore{};
        }
    };


    template<>
    struct DeviceScoring<VkPhysicalDeviceFeatures> {
        //scoredtype just finna help with copy paste a bit
        using ScoredType = VkPhysicalDeviceFeatures;
        DeviceScore operator()(ScoredType const& features) const noexcept{
            using DefW = DefaultWeights<ScoredType>;

            DeviceScore ret{true, 0};
#ifdef EWE_USING_REFLECTION
            template for(constexpr auto member : std::define_static_array(std::meta::nonstatic_data_members_of(^^DefW))) {
                if(![:member:].CheckRequirement())
            }
#else
            if(!DefW::samplerAnisotropy.CheckRequirement(features.samplerAnisotropy)){
                return DeviceScore(false, 0);
            }
            else{
                ret.score += DefW::samplerAnisotropy.score * features.samplerAnisotropy;
            }
#endif
            return ret;
        }
    };
/* copy paste source
    template<>
    struct DeviceScoring<VkPhysicalDeviceProperties>{
        using ScoredType = VkPhysicalDeviceProperties;
        DeviceScore operator()(ScoredType const& features) const noexcept{
            using DefW = DefaultWeights<ScoredType>;

        }
    };
*/
    template<>
    struct DeviceScoring<VkPhysicalDeviceProperties>{
        using ScoredType = VkPhysicalDeviceProperties;
        DeviceScore operator()(ScoredType const& features) const noexcept{
            using DefW = DefaultWeights<ScoredType>;
            return {};
        }
    };

    template<typename T>
    concept HasDeviceScoring = requires {!std::is_void_v<typename DeviceScoring<T>::ScoredType>;};
}//namespace EWE