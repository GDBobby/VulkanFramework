#pragma once

#include "EightWinds/VulkanHeader.h"

#include <vector>
#include <concepts>

namespace EWE{

    template <typename T>
    concept VulkanStruct = requires(T t) {
        { t.sType } -> std::convertible_to<VkStructureType>;
        { t.pNext } -> std::convertible_to<const void*>;
    };

    enum class WeightComparison{
        minimum,
        match,
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
    

        constexpr DeviceWeight(WeightType required, uint64_t score) : required{required}, score{score} {}
    };

    template<typename T>
    struct DefaultWeights{};

    template<>
    struct DefaultWeights<VkPhysicalDeviceFeatures>{
        //i might just define it here so I can use it later
        //static constexpr uint32_t sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2; 

        static constexpr auto samplerAnisotropy = DeviceWeight<bool>(true, 1 << 8);

    };

    template<>
    struct DefaultWeights<VkPhysicalDeviceProperties>{
        static constexpr auto maxImageDimension1D = DeviceWeight<uint32_t>(256, 3); //each additional texel allowed in this dimensions adds the second parameter (3) to the score
    };

    struct DeviceScore{
        bool metRequirements;
        uint64_t score;
    };

    template<typename T>
    struct DeviceScoring{};


    template<>
    struct DeviceScoring<VkPhysicalDeviceFeatures> {
        //scoredtype just finna help with copy paste a bit
        using ScoredType = VkPhysicalDeviceFeatures;
        DeviceScore operator()(ScoredType const& features) const noexcept{
            using DefW = DefaultWeights<ScoredType>;
            /*
            C++ 26 reflection

            for(auto& default_member : DefW::static_constexpr_members){
                auto& input_member = find(features.member); //find would match the names
                if(default_member.required && !input_member){
                    return DeviceScore(false, 0);
                }
                else{
                    if constexpr (std::is_same_v<input_member, bool>){
                        ret.score += default_member.score;
                    }
                    else {
                        ret.score += input_member * default_member.score; //casting if necessary
                    }
                }
            }
            */


            DeviceScore ret{true, 0};
            if(!DefW::samplerAnisotropy.CheckRequirement(features.samplerAnisotropy)){
                return DeviceScore(false, 0);
            }
            else{
                ret.score += DefW::samplerAnisotropy.score * features.samplerAnisotropy;
            }


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

        }
    };

    template<typename T>
    concept HasDeviceScoring = requires {typename DeviceScoring<T>;};
    
    template<HasDeviceScoring... Args>
    struct ScoreInput{
        std::tuple<Args...> values;

        template<typename T>
        constexpr auto& get() noexcept {
            return std::get<T>(values);
        }    
        template<typename T>
        constexpr const auto& get() const noexcept {
            return std::get<T>(values);
        }

    private:

        //helper function for GetTotalScore
        template<typename T>
        constexpr void AccumulateScore(DeviceScore& total) const noexcept {
            const auto& scorer = get<T>(values);
            DeviceScore s = DeviceScoring<T>()(scorer);
            if (!s.metRequirements){
                total.metRequirements = false;
            }
            total.score += s.score;
        }
    public:
        constexpr DeviceScore GetTotalScore() const noexcept {
            DeviceScore total{true, 0};

            (AccumulateScore<Args>(total), ...);

            return total;
        }

    };
}//namespace EWE