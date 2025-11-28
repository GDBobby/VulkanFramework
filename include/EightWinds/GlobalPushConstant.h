#pragma once

#include "EightWinds/VulkanHeader.h"

#include "EightWinds/Buffer.h"
#include "EightWinds/Image.h"

namespace EWE{

    struct GlobalPushConstant{
        static constexpr uint8_t buffer_count = 8;
        //for DBAs (device buffer address), 0 is invalid, and means the buffer isn't used
        VkDeviceAddress buffer_addr[buffer_count];

        static constexpr uint8_t texture_count = 8;
        //for texture indices, negative values are invalid (typically -1)
        //and means the texture slot isn't being used
        int texture_indices[texture_count];
        
        //guaranteed to have another 32 bytes
        //potentially more data space available



        //i think i could just have a single large mat4 buffer?
        //or like 2 or 3, one stable, one rewritten every frame
        //i could house transforms in that, potentially some other mat4s


        void Bind(uint8_t slot, Buffer const& buffer) noexcept {
            assert(slot < buffer_count);
            buffer_addr[slot] = buffer.deviceAddress;
        }

        void Reset() noexcept {
            //set all buffer addresses to 0
            for(uint8_t i = 0; i < buffer_count; i++){
                buffer_addr[i] = 0;
            }
            for(uint8_t i = 0; i < texture_count; i++){
                texture_indices[i] = -1;
            }
        }
    };
}