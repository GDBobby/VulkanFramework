#include "EightWinds/VulkanHeader.h"
#include "PhysicalDevice.h"

namespace EWE{

    struct QueueFamily {
        vk::QueueFamilyProperties2 properties; //includes properties1

        [[nodiscard]] explicit QueueFamily(vk::QueueFamilyProperties2 const& properties);

        bool SupportsGraphics() const;
        bool SupportsCompute() const;
        bool SupportsTransfer() const;
        bool SupportsSurfacePresent(vk::SurfaceKHR surface) const;

        static std::vector<QueueFamily> Enumerate(vk::PhysicalDevice device);
    };

    struct Queue {
        QueueFamily& family;
        vk::Queue queue;

        operator vk::Queue() const { return queue; }
    };
    struct QueueRequest {
        enum class Level {
            Luxury, //no warning
            Preferred, //warning if unavailable
            Vital, //error if unavailable
        };

        //idk why QueueFlagBits doesnt include present, but whatever. 
        //redefining QueueFlagBits here so I can add present
        //i also need it to be able to combine
        enum Flags : uint8_t { 
            Graphics,
            Present,
            GraphicsPresentCombined,
            Transfer,
            Compute,
            TransferComputeCombined,
        };

        Level level;
        Flags flags;

        static std::vector<std::optional<Queue>> RequestQueues(std::span<QueueRequest> requests, vk::SurfaceKHR surface);
    };

}

//vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR