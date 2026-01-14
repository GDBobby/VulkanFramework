#include "EightWinds/GlobalPushConstant.h"

#include <cassert>

namespace EWE{

    void GlobalPushConstant_Raw::Bind(uint8_t slot, Buffer const& buffer) noexcept {
        assert(slot < buffer_count);
        buffer_addr[slot] = buffer.deviceAddress;
    }

    void GlobalPushConstant_Raw::Reset() noexcept {
        //set all buffer addresses to 0
        for(uint8_t i = 0; i < buffer_count; i++){
            buffer_addr[i] = 0;
        }
        for(uint8_t i = 0; i < texture_count; i++){
            texture_indices[i] = -1;
        }
    }

    void GlobalPushConstant_Abstract::UpdateBuffer(uint8_t frameIndex) {
        auto& push = deferred_push->GetRef(frameIndex);
        for (uint8_t i = 0; i < GlobalPushConstant_Raw::buffer_count; i++) {
            if (buffers[i] == nullptr) {
                break; //if not allowing gaps, break
            }
            else {
                push.buffer_addr[i] = buffers[i]->deviceAddress;
            }
        }
        for (uint8_t i = 0; i < GlobalPushConstant_Raw::texture_count; i++) {
            if (textures[i] == nullptr) {
                break; //if not allowing gaps, break
            }
            else {
                push.texture_indices[i] = textures[i]->index;
            }
        }
    }
    void GlobalPushConstant_Abstract::UpdateBuffer() {
        for (uint8_t i = 0; i < max_frames_in_flight; i++) {
            UpdateBuffer(i);
        }
    }
}// namespace EWE