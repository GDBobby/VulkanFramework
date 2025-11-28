#pragma once

#include "EightWinds/VulkanHeader.h"

#include "EightWinds/Backend/RenderInfo.h"
#include "EightWinds/Command/Execute.h"

#include "EightWinds/Command/CommandPool.h"

#include <span>

//equivalent a renderpass subpass?

namespace EWE{
    struct Buffer;
    struct ImageInfo;
    struct GlobalPushConstant;

    struct PushTracker{
        Resource<Buffer>* buffers[GlobalPushConstant::buffer_count];
        Resource<ImageInfo>* images[GlobalPushConstant::texture_count];
    };

    template<typename T>
    struct Resource{
        T* resource;
        bool writes;
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

        std::vector<PushTracker> pushTrackers{};
        //for easier tracking
        std::vector<Resource<Buffer>*> usedBuffers{};
        std::vector<Resource<ImageInfo>*> usedImages{};

        //i'd like writes to be statically reflected but i dont think thats possible right now
        //it should be somewhat easy to fix this in the future
        //if necessary, i could leave this and put another function that statically reflects to reduce refactorign cost
        void UseBuffer(Buffer* buffer, GlobalPushConstant* pushPtr, uint8_t slot, bool writes) noexcept;
        //if the user passes in a index greater/equal to GlobalPushConstant::buffer_count then i'll shift it by that value
        //ill also assert both (buffer and image) slots are valid in debug mode
        void UseImage(ImageInfo* buffer, GlobalPushConstant* pushPtr, uint8_t slot, bool writes) noexcept;

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