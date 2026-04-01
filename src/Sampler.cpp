#include "EightWinds/Sampler.h"

namespace EWE{
    Sampler::Sampler(LogicalDevice& _logicalDevice, VkSamplerCreateInfo const& samplerInfo)
        : logicalDevice{_logicalDevice},
        info{samplerInfo}
    {
        EWE_VK(vkCreateSampler, logicalDevice, &info, nullptr, &sampler);
    }

    Sampler::~Sampler() {
        logicalDevice.garbageDisposal.Toss(sampler, VK_OBJECT_TYPE_SAMPLER);
    }

    Sampler::CondensedType Sampler::Condense(VkSamplerCreateInfo const& info){
        //we can ignore flags
        uint64_t ret = 0;
    
        std::size_t current_bit_placement = 0;

        auto AddFilter = [](uint64_t& _ret, std::size_t& _current_bit_placement, VkFilter filter){
            uint64_t temp = 0;
            switch(filter){
                case VK_FILTER_NEAREST: temp = 0b01; break;
                case VK_FILTER_LINEAR: temp = 0b10; break;
                case VK_FILTER_CUBIC_EXT: temp = 0b11; break;
                default: EWE_UNREACHABLE;
            }
            _ret |= temp << _current_bit_placement;
            _current_bit_placement += 2;
        };
        AddFilter(ret, current_bit_placement, info.magFilter);
        AddFilter(ret, current_bit_placement, info.minFilter);

        ///mipmap mode is a boolean
        ret |= info.mipmapMode << current_bit_placement;
        current_bit_placement++;

        auto address_mode_condenser = [](VkSamplerAddressMode const& am){
            switch(am){
                case VK_SAMPLER_ADDRESS_MODE_REPEAT: return 0b000;
                case VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT: return 0b001;
                case VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE: return 0b010;
                case VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER: return 0b011;
                case VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE: return 0b100;
                default: EWE_UNREACHABLE;
            }
        };

        ret |= address_mode_condenser(info.addressModeU) << current_bit_placement;
        current_bit_placement += 3;
        ret |= address_mode_condenser(info.addressModeV) << current_bit_placement;
        current_bit_placement += 3;
        ret |= address_mode_condenser(info.addressModeW) << current_bit_placement;
        current_bit_placement += 3;

        //i'll put all the floats in 1 area

        ret |= (info.anisotropyEnable == VK_TRUE ? 1 : 0) << current_bit_placement;
        current_bit_placement++;
        ret |= (info.compareEnable == VK_TRUE ? 1 : 0) << current_bit_placement;
        current_bit_placement++;

        ret |= info.compareOp << current_bit_placement;
        current_bit_placement += 3;

        if(info.borderColor <= 5){
            //the custom colors will bug this, so it's been defaulted to TRANSPARENT_BLACK
            ret |= info.borderColor << current_bit_placement; 
            current_bit_placement += 3;
        }

        ret |= (info.unnormalizedCoordinates == VK_TRUE ? 1 : 0) << current_bit_placement;
        current_bit_placement++;

        //i believe i can condense the 4 floats to a uint8_t, and im currently at 23 bits
        uint8_t minLod = static_cast<uint8_t>(info.minLod);
        uint8_t maxLod = static_cast<uint8_t>(info.maxLod);
        float normalized_lodBias = (info.mipLodBias - info.minLod) / (info.maxLod - info.minLod);
        if(normalized_lodBias < 0.f){
            normalized_lodBias = 0.f;
        }
        else if (normalized_lodBias > 1.f){
            normalized_lodBias = 1.f;
        }
        uint8_t mipLodBias = static_cast<uint8_t>(normalized_lodBias * 255.0f);

        uint8_t maxAni = static_cast<uint8_t>(info.maxAnisotropy);

        //if i keep it uint8 and shift it, i'll get math errors. so i need to re-cast it
        auto float_emplacer = [](uint64_t& _ret, uint8_t condensed_float, std::size_t& _current_bit_placement){
            uint64_t float_buffer = condensed_float;
            _ret |= float_buffer << _current_bit_placement;
            _current_bit_placement += 8;
        };
        float_emplacer(ret, mipLodBias, current_bit_placement);
        float_emplacer(ret, maxAni, current_bit_placement);
        float_emplacer(ret, minLod, current_bit_placement);
        float_emplacer(ret, maxLod, current_bit_placement);

        return ret;
    }
    VkSamplerCreateInfo Sampler::Expand(CondensedType condensed){

        auto ExpandFilter = [](CondensedType buffer, uint8_t bit_shift) -> VkFilter{
            const auto temp = (buffer >> bit_shift) & 0b11;
            switch(temp){
                case 0b01: return VK_FILTER_NEAREST;
                case 0b10: return VK_FILTER_LINEAR;
                case 0b11: return VK_FILTER_CUBIC_EXT;
                default: EWE_UNREACHABLE;
            }
        };

        auto address_mode_expander = [](CondensedType buffer, uint8_t bit_shift) -> VkSamplerAddressMode{
            const auto temp = (buffer >> bit_shift) & 0b111;
            switch(temp){
                case 0b000: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
                case 0b001: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
                case 0b010: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                case 0b011: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
                case 0b100: return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
                default: EWE_UNREACHABLE;
            }
        };

        const float maxAni = static_cast<float>((condensed >> 31) & 0xFF);
        const float minLod = static_cast<float>((condensed >> 39) & 0xFF);
        const float maxLod = static_cast<float>((condensed >> 47) & 0xFF);
        const float normalized_lodBias = static_cast<float>((condensed >> 47) & 0xFF) / 255.0f;
        const float mipLodBias = normalized_lodBias * (maxLod - minLod) + minLod;

        //all then floats, for bit shifting
        return VkSamplerCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .magFilter = ExpandFilter(condensed, 0),
            .minFilter = ExpandFilter(condensed, 2),
            .mipmapMode = static_cast<VkSamplerMipmapMode>((condensed >> 4) & 1),
            .addressModeU = address_mode_expander(condensed, 5),
            .addressModeV = address_mode_expander(condensed, 8),
            .addressModeW = address_mode_expander(condensed, 11),
            .mipLodBias = mipLodBias,
            .anisotropyEnable = ((condensed >> 14) & 1) ? VK_TRUE : VK_FALSE,
            .maxAnisotropy = maxAni,
            .compareEnable = ((condensed >> 15) & 1) ? VK_TRUE : VK_FALSE,
            .compareOp = static_cast<VkCompareOp>((condensed >> 16) & 0b111),
            .minLod = minLod,
            .maxLod = maxLod,
            .borderColor = static_cast<VkBorderColor>((condensed >> 19) & 0b111),
            .unnormalizedCoordinates = ((condensed >> 22) & 1) ? VK_TRUE : VK_FALSE,
        };
    }
}