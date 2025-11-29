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
    struct ImageInfo;
    struct GlobalPushConstant;


    template<typename T>
    struct Resource{
        T* resource;
        bool writes;
        //reference count kinda. im not sure if ill ever use this but conceptually its good
        uint8_t usageInCurrentTask = 1; 
    };
    struct PushTracker{
        GlobalPushConstant* pushAddress;
        Resource<Buffer>* buffers[GlobalPushConstant::buffer_count];
        Resource<ImageInfo>* textures[GlobalPushConstant::texture_count];

        [[nodiscard]] PushTracker(GlobalPushConstant* ptr) noexcept;

        //to simplify usage a bit i could use function pointers here, and let the players mess 
        //with pushtracker directly, instead of going thru GPUTask
    };

    //the id of this task is its address
    struct GPUTask{
        LogicalDevice& logicalDevice;
        //i think i define a command pool here

        //GPUTask* feedsInto = nullptr; //if its nullptr its a present/submission

        CommandExecutor commandExecutor;

        //i need another system wrapping GPUTask to handle how the command buffers are dealt with
        void Execute(CommandBuffer& cmdBuf){
            commandExecutor.Execute(cmdBuf);
        }

        //write directly to the push tracker. 
        //replace the old buffer if necessary
        std::vector<PushTracker> pushTrackers{};
        //for easier tracking
        std::vector<Resource<Buffer>*> usedBuffers{};
        std::vector<Resource<ImageInfo>*> usedImages{};

        //i'd like writes to be statically reflected but i dont think thats possible right now
        //it should be somewhat easy to fix this in the future
        //if necessary, i could leave this and put another function that statically reflects to reduce refactorign cost
       
        //if it was a small amount (100 or less) of pushes, using the address and just counting the offset into pushtrackers 
        //every time might be better, but we could be talking 10k+ pushes
        //only tradeoff here is that deferred push needs to be a full specialization of DeferredPointer
        void UseBuffer(Buffer* buffer, uint32_t pushIndex, uint8_t slot, bool writes) noexcept;
        //if the user passes in a index greater/equal to GlobalPushConstant::buffer_count then i'll shift it by that value
        //ill also assert both (buffer and image) slots are valid in debug mode
        void UseImage(ImageInfo* buffer, uint32_t pushIndex, uint8_t slot, bool writes) noexcept;

        //idk if i want to commit to renderInfo here yet
        //std::optional<RenderInfo> renderInfo;

        [[nodiscard]] explicit GPUTask(LogicalDevice& logicalDevice) 
            : logicalDevice{logicalDevice}, commandExecutor{logicalDevice} 
        {}
        ~GPUTask();
        GPUTask(GPUTask const&) = delete;
        GPUTask& operator=(GPUTask const&) = delete;

        //i dont know if i want it to be movable or not yet, but for now this is fine
        GPUTask(GPUTask&&) = default;
        GPUTask& operator=(GPUTask&&) = delete;

        //im not committed to putting the command buffer here. 
        //i might let each GPUTask create its own command buffer on execution
        //void Execute(CommandBuffer& cmdBuf) const noexcept;
    };
}