#pragma once

#include "EightWinds/Buffer.h"

namespace EWE{
    Buffer::Buffer(){
        
    }

    Buffer::~Buffer(){
        if(vmaAlloc != VK_NULL_HANDLE){
            vmaDestroyBuffer(logicalDevice.vmaAllocator, buffer_info.buffer, vmaAlloc);
        }
    }
}