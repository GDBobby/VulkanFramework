#pragma once

#include "EightWinds/Queue.h"
#include "EightWinds/CommandPool.h"
#include "EightWinds/CommandBuffer.h"
#include "EightWinds/Backend/Semaphore.h"
#include "EightWinds/Backend/StagingBuffer.h"

#include "EightWinds/RenderGraph/SubmissionTask.h"
#include "EightWinds/RenderGraph/Resources.h"

#include "EightWinds/Image.h"
#include "EightWinds/Buffer.h"

#include "EightWinds/Data/RuntimeArray.h"

#include <vector>

namespace EWE{


    struct SingleTimeCommand {
        CommandPool& cmdPool;
        CommandBuffer cmdBuf;
        TimelineSemaphore& semaphore;
        [[nodiscard]] explicit SingleTimeCommand(CommandPool& _cmdPool, TimelineSemaphore& _semaphore) 
        : cmdPool { _cmdPool }, 
            cmdBuf{ cmdPool.AllocateCommand(VK_COMMAND_BUFFER_LEVEL_PRIMARY) },
            semaphore{_semaphore}
        {} //construct the commandbuffer here
        SingleTimeCommand(SingleTimeCommand const& copySrc) = delete;
        SingleTimeCommand& operator=(SingleTimeCommand const& copySrc) = delete;
        SingleTimeCommand(SingleTimeCommand&& moveSrc) noexcept;
        SingleTimeCommand& operator=(SingleTimeCommand&& moveSrc);
    };

    template<ResourceType Resource>
    struct STC_Sub_Package;

    template<>
    struct STC_Sub_Package<Image>{
        std::vector<Image::Barrier> barriers;
        std::vector<Image*> images;
        VkImageLayout layout;
        Queue* dstQueue;
    };
    template<>
    struct STC_Sub_Package<Buffer>{
        std::vector<Buffer::Barrier> barriers;
        std::vector<Buffer*> bufs;
        Queue* dstQueue;
    };


	template<ResourceType Resource>
	struct TransferContext{
		//i want the context to describe the entire process, and to return data required for callbacks
	};
	template<>
	struct TransferContext<Image>{
		RuntimeArray<Image*> images; //set the dstlayout here
		RuntimeArray<VkBufferImageCopy> regions;

		StagingBuffer stagingBuffer;

		UsageData<Image> final_usage;
		bool generatingMipMaps;

		//should always be equal to image count

		void Acquire(CommandBuffer& cmdBuf, Queue& acqQueue, UsageData<Image> const& initial_usage);
		void Stage(CommandBuffer& cmdBuf, UsageData<Image> const& initial_usage);
		RuntimeArray<VkImageMemoryBarrier2> ChangeOwnership(CommandBuffer& cmdBuf, Queue& acqQueue, Queue& rh_queue, UsageData<Image> const& initial_usage);
	};
	template<>
	struct TransferContext<Buffer>{
		RuntimeArray<Buffer*> buffers;

		//potentially size of 1 and not equal to size of resource, 
		// if a buffer is going to be initialized per flight with 1 set of data
		RuntimeArray<VkBufferCopy> regions;

		StagingBuffer stagingBuffer;

		UsageData<Buffer> final_usage;

		void Acquire(CommandBuffer& cmdBuf, Queue& acqQueue, UsageData<Buffer> const& initial_usage);
		void Stage(CommandBuffer& cmdBuf);
		RuntimeArray<VkBufferMemoryBarrier2> ChangeOwnership(CommandBuffer& cmdBuf, Queue& acqQueue, Queue& dstQueue, UsageData<Buffer> const& initial_usage);
	};

} //namespace EWE