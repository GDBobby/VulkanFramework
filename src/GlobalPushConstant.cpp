#include "EightWinds/VulkanHeader.h"

namespace EWE{

    void GlobalPushConstant::Bind(uint8_t slot, Buffer const& buffer) noexcept {
        assert(slot < buffer_count);
        buffer_addr[slot] = buffer.deviceAddress;
    }

    void GlobalPushConstant::Reset() noexcept {
        //set all buffer addresses to 0
        for(uint8_t i = 0; i < buffer_count; i++){
            buffer_addr[i] = 0;
        }
        for(uint8_t i = 0; i < texture_count; i++){
            texture_indices[i] = -1;
        }
    }
}// namespace EWE