#pragma once

#include "EightWinds/VulkanHeader.h"

#include "EightWinds/Buffer.h"
#include "EightWinds/Image.h"

#include "EightWinds/DescriptorImageInfo.h"

#include "EightWinds/RenderGraph/Command/InstructionPointer.h"

namespace EWE{

    struct GlobalPushConstant_Raw{
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


        void Bind(uint8_t slot, Buffer const& buffer) noexcept;

        void Reset() noexcept;
    };
    
    struct GlobalPushConstant_Abstract{
        std::array<Buffer*, GlobalPushConstant_Raw::buffer_count> buffers;
        std::array<DescriptorImageInfo*, GlobalPushConstant_Raw::texture_count> textures;
        
        InstructionPointer<GlobalPushConstant_Raw>* deferred_push = nullptr;

        [[nodiscard]] GlobalPushConstant_Abstract() {
            buffers.fill(nullptr);
            textures.fill(nullptr);
        }

        //updates the Execute::ParamPool push constant
        void UpdateBuffer(uint8_t frameIndex);
        void UpdateBuffer();
    };
}