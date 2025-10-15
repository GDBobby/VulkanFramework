#include "EightWinds/VulkanHeader.h"
#include "EightWinds/PhysicalDevice.h"

namespace EWE{

    struct Device{
        PhysicalDevice physicalDevice;
        vk::Device device;
    };
}