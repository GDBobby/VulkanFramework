#pragma once
#include "EightWinds/VulkanHeader.h"

//#include <span>
//#include <optional>

namespace EWE{

//vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR
    struct QueueFamily {
        VkQueueFamilyProperties properties; //2 includes properties1, but I dont' really see the point in 2
        bool supportsSurface;

        uint32_t index; //queue family index

        [[nodiscard]] explicit QueueFamily(uint32_t index, VkQueueFamilyProperties const& properties, bool supportsSurface);

        bool SupportsGraphics() const;
        bool SupportsCompute() const;
        bool SupportsTransfer() const;
        bool SupportsSurfacePresent() const;

        //for headless, not supporting
        //[[nodiscard]] explicit QueueFamily(PhysicalDevice& physicalDevice, uint8_t index, VkQueueFamilyProperties const& properties);
        //static std::vector<QueueFamily> Enumerate(PhysicalDevice& physicalDevice);
    };

   
} //namespace EWE

 /*
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
#undef QBF_REF
            ePresent = eOpiticalFlowNV << 1,

            COUNT = 8 //the amount of flags
        };

        Level level;
        Flags flags;
        MergePreference mergePref;

        static std::vector<std::optional<Queue>> RequestQueues(std::span<QueueRequest> requests, vkSurfaceKHR surface);
    };
    */
