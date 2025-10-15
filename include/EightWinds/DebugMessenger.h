#pragma once
#include "EightWinds/VulkanHeader.h"

#include "Instance.h"

namespace EWE{
    class DebugMessenger {
        DebugMessenger(Instance& instance);

        vk::DebugUtilsMessengerEXT debugMessenger;

        Instance& instance;
    };
}