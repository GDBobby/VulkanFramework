#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/RenderGraph/GPUTask.h"



namespace EWE{

    struct ResourceCollectionSmall;//hide this maybe

    template<typename T>
    struct BarrierResource {
        T* resource;
    };
    template<>
    struct BarrierResource<Buffer> {
        Buffer* resource;//the ownign queue might change
    };
    template<>
    struct BarrierResource<Image> {
        Image* resource; //need to change the layout
        VkImageLayout finalLayout;
    };

    struct TaskBridge{
        LogicalDevice& logicalDevice;
        GPUTask* lhs;
        GPUTask* rhs; //rhs depends on lhs. the alternatives are to name this predecesor and successor which i dont like much
        std::string name; //"{lhs->name} -> {rhs->name}"

        [[nodiscard]] explicit TaskBridge(GPUTask& lhs, GPUTask& rhs) noexcept;
        [[nodiscard]] explicit TaskBridge(GPUTask& rhs) noexcept;

        TaskBridge(TaskBridge const& copySrc) = delete;
        TaskBridge(TaskBridge&& moveSrc) noexcept;

        TaskBridge& operator=(TaskBridge const& copySrc) = delete;
        TaskBridge& operator=(TaskBridge&& copySrc) = delete;

        std::vector<VkBufferMemoryBarrier2> bufferBarriers{};
        std::vector<VkImageMemoryBarrier2> imageBarriers{};
        std::vector<BarrierResource<Buffer>> bufferBarrierResources{};
        std::vector<BarrierResource<Image>> imageBarrierResources{};

		VkDependencyInfo dependencyInfo{};

        void RecreateBarriers(const uint8_t frameIndex);

        void Execute(CommandBuffer& cmdBuf);

        void LeftToRightBarriers(const uint8_t frameIndex, ResourceCollectionSmall& rhsColl);
        void RightOnlyBarriers(const uint8_t frameIndex, ResourceCollectionSmall& rhsColl);
    };
}