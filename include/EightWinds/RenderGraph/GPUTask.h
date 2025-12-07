#pragma once

#include "EightWinds/VulkanHeader.h"

#include "EightWinds/Backend/RenderInfo.h"
#include "EightWinds/Command/Execute.h"

#include "EightWinds/Command/CommandPool.h"

#include "EightWinds/GlobalPushConstant.h"

#include <span>

//equivalent a renderpass subpass?

namespace EWE{
    struct Buffer;
    struct Image;
    struct CommandRecord;

    struct ResourceUsageData {
        VkPipelineStageFlags2 stage;
        VkAccessFlags2 accessMask;
    };
    template<typename T>
    struct Resource{
        T* resource;
        ResourceUsageData usage;
    };

    template<>
    struct Resource<Buffer> {
        Buffer* resource;
        ResourceUsageData usage;
    };
    template<>
    struct Resource<Image> {
        Image* resource;
        ResourceUsageData usage;
        //add validation to make sure layout doesnt change within a task
        VkImageLayout layout;
    };

    //ok i think i remove the trackers. they just add an intermediary overhead. i can do a direct comparison on the param pool
    struct PushTracker{
        GlobalPushConstant* pushAddress;
        Resource<Buffer> buffers[GlobalPushConstant::buffer_count];
        Resource<Image> textures[GlobalPushConstant::texture_count];

        [[nodiscard]] PushTracker(GlobalPushConstant* ptr) noexcept;

        //to simplify usage a bit i could use function pointers here, and let the players mess 
        //with pushtracker directly, instead of going thru GPUTask
    };
    struct BlitTracker {
        Resource<Image> srcImage;
        Resource<Image> dstImage;
    };
    struct RenderTracker {
        RenderInfo vk_data;
        RenderInfo2 compact;
    };
    

    //the id of this task is its address
    //GPUTask is intended to be used in a single thread.
    //if its desired to multi-thread within a single task, sync needs to be external
    //i need to be able to insert barriers
    struct GPUTask{
        LogicalDevice& logicalDevice;
        //i think i define a command pool here, or at least a queue
        Queue& queue;

        std::string name;
        CommandExecutor commandExecutor;
        
        [[nodiscard]] explicit GPUTask(LogicalDevice& logicalDevice, Queue& queue, CommandRecord& cmdRecord, std::string_view name);
        ~GPUTask();
        GPUTask(GPUTask const&) = delete;
        GPUTask& operator=(GPUTask const&) = delete;
        GPUTask(GPUTask&&) = delete;
        GPUTask& operator=(GPUTask&&) = delete;

        //i need another system wrapping GPUTask to handle how the command buffers are dealt with
        void Execute(CommandBuffer& cmdBuf);

        //optional, a compute wouldnt use htis
        RenderTracker* renderTracker = nullptr;

        void SetRenderInfo(); 
        void UpdateFrameIndex(uint8_t frameIndex);
        
        //write directly to the push tracker. 
        //replace the old buffer if necessary
        std::vector<PushTracker> pushTrackers{};
        std::vector<BlitTracker> blitTrackers{};

        //i'd like writes to be statically reflected but i dont think thats possible right now
        //it should be somewhat easy to fix this in the future
        //if necessary, i could leave this and put another function that statically reflects to reduce refactorign cost
       
        //if it was a small amount (100 or less) of pushes, using the address of the push itself and
        //counting the offset into pushtrackers every time might be better
        //but we could be talking 10k+ pushes
        void PushBuffer(Buffer* buffer, uint32_t pushIndex, uint8_t slot, ResourceUsageData const& usageData) noexcept;
        //if the user passes in a index greater/equal to GlobalPushConstant::buffer_count then i'll shift it by that value
        //ill also assert both (buffer and image) slots are valid in debug mode
        void PushImage(Image* image, uint32_t pushIndex, uint8_t slot, ResourceUsageData const& usageData, VkImageLayout layout) noexcept;

        //if its nullptr, it's guaranteed no barriers are necessary
        void DefineBlitUsage(uint16_t blitIndex, Image* srcImage, Image* dstImage) noexcept;
    };
}