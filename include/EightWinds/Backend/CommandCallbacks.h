#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/CommandBuffer.h"
#include "EightWinds/Backend/StagingBuffer.h"
#include "EightWinds/Image.h"
#include "EightWinds/Backend/Barrier.h"

namespace EWE {
	struct GraphicsCommand {
        CommandBuffer* command{ nullptr };
        ImageInfo* imageInfo{ nullptr };
        StagingBuffer* stagingBuffer{ nullptr }; 
        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    };

    struct TransferCommand {
        std::vector<CommandBuffer*> commands;
        std::vector<StagingBuffer*> stagingBuffers;
        std::vector<PipelineBarrier> pipeBarriers;
        std::vector<Image*> images;
        Semaphore* semaphore;

        TransferCommand() : commands{}, stagingBuffers{}, pipeBarriers{}, images{}, semaphore{ nullptr } {} //constructor
        TransferCommand(TransferCommand& copySource); //copy constructor
        TransferCommand& operator=(TransferCommand& copySource); //copy assignment
        TransferCommand(TransferCommand&& moveSource) noexcept;//move constructor
        TransferCommand& operator=(TransferCommand&& moveSource) noexcept; //move assignment
        //TransferCommand& operator+=(TransferCommand& copySource);
    };
}