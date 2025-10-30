#include "EightWinds/VulkanHeader.h"
#include "PhysicalDevice.h"

#include <span>
#include <optional>

namespace EWE{

    struct QueueFamily {
        vk::QueueFamilyProperties properties; //includes properties1

        uint16_t index; //queue family index

        [[nodiscard]] explicit QueueFamily(vk::QueueFamilyProperties const& properties);

        bool SupportsGraphics() const;
        bool SupportsCompute() const;
        bool SupportsTransfer() const;
        bool SupportsSurfacePresent(vk::SurfaceKHR surface) const;

        static std::vector<QueueFamily> Enumerate(vk::PhysicalDevice device);
    };

    struct Queue {

        QueueFamily& family;
        vk::Queue queue;


		//void BeginLabel(const char* name, float red, float green, float blue);
		//void EndLabel();

        operator vk::Queue() const { return queue; }
    };
    struct QueueRequest {
        enum class Level {
            None,
            Warning,
            Error
        };

        enum class MergePreference{ //based on Level, youll get feedback if you dont get the preferred Merge
            Combined,
            Separate,
        };

        //idk why QueueFlagBits doesnt include present, but whatever. 
        //redefining QueueFlagBits here so I can add present
        //i also need it to be able to combine, which an enum class isn't very friendly with
        enum Flags : uint8_t { 
#define QFB_REF(a) a = QueueFlagBits::a
            QBF_REF(eGraphics),
            QBF_REF(eCompute),
            QBF_REF(eTransfer),
            QBF_REF(eSparseBinding),
            QBF_REF(eProtected),
            QBF_REF(eVideoDecodeKHR),
            QBF_REF(eOpticalFlowNV)
#undef
            ePresent = eOpiticalFlowNV << 1,

            COUNT = 8 //the amount of flags
        };

        Level level;
        Flags flags;
        MergePreference mergePref;

        static std::vector<std::optional<Queue>> RequestQueues(std::span<QueueRequest> requests, vk::SurfaceKHR surface);
    };

}

//vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR