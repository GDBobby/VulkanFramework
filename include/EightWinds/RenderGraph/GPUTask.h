#pragma once

#include "EightWinds/VulkanHeader.h"

#include "EightWinds/Backend/RenderInfo.h"
#include "EightWinds/RenderGraph/Command/Execute.h"

#include "EightWinds/Command/CommandPool.h"

#include "EightWinds/GlobalPushConstant.h"

#include <span>

//equivalent a renderpass subpass?

namespace EWE{
    struct Buffer;
    struct Image;
    struct CommandRecord;
    
    template<typename T>
    struct Resource{};

    struct BufferUsageData {
        VkPipelineStageFlags2 stage;
        VkAccessFlags2 accessMask;
    };
    struct ImageUsageData{
        VkPipelineStageFlags2 stage;
        VkAccessFlags2 accessMask;
        VkImageLayout layout;
    };

    template<>
    struct Resource<Buffer> {
        Buffer* buffer;
        BufferUsageData usage;
    };
    template<>
    struct Resource<Image> {
        Image* image;
        ImageUsageData usage;
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
        
        //these are debug helpers, remove for the moment
        //std::vector<PushTracker> pushTrackers{};
        //std::vector<BlitTracker> blitTrackers{};

        //add validation here to make sure each resource is unique
        int AddImagePin(Image* image, VkPipelineStageFlags2 stage, VkAccessFlags2 accessMask, VkImageLayout layout);
        int AddImagePin(Image* image, ImageUsageData const& usage);
        int AddBufferPin(Buffer* buffer, BufferUsageData const& usage);
        int AddBufferPin(Buffer* buffer, VkPipelineStageFlags2 stage, VkAccessFlags2 accessMask);

        void SetResource(int pin, Image& image);
        void SetResource(int pin, Buffer& buffer);

        std::vector<Resource<Image>*> explicitImageState;
        std::vector<Resource<Buffer>*> explicitBufferState;
    };
}