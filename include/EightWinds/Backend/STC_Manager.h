#pragma once

#include "EightWinds/Data/KeyValueContainer.h"
#include "EightWinds/VulkanHeader.h"
#include "EightWinds/Fence.h"

#include "EightWinds/"

#include <cassert>
#include <thread>
#include <mutex>
#include <array>
#include <vector>


/*
* std::this_thread::sleep_for(std::chrono::microseconds(1));
    this is a small amount of time, 
    and most likely 
    it'll allow the OS to take over the thread and do other operations on it, 
    then return control of this thead to our program.
    It's not 100% for the OS to take over, but if it does, the thread will potentially sleep for longer (up to multiple milliseconds)

    it is possible to spin the thread until a certain amount of time has passed
    BUT if immediate control of a new fence/semaphore is required,
    it's recommended throw an error if the requested data is not available, and increase pool size as needed
*/

namespace EWE{

    struct GraphicsFence {
        Fence fence{};
        std::mutex mut{};
        GraphicsCommand gCommand{};
        Semaphore* signalSemaphore{ nullptr };
        void CheckReturn(uint64_t time);
    };

    class QueueSyncPool{
    private:
        const uint16_t size;

        std::vector<Semaphore> semaphores;
        std::mutex semAcqMut{};

        KeyValueContainer<std::thread::id, ThreadedSingleTimeCommands> threadedSTCs;
        static thread_local ThreadedSingleTimeCommands* threadSTC;
        //std::mutex threadedSTCMutex{};

        std::vector<Fence> fences;
        std::mutex fenceAcqMut{};
        std::vector<GraphicsFence> mainThreadGraphicsFences;
        VkCommandPool mainThreadSTCGraphicsPool{ VK_NULL_HANDLE };
        std::vector<CommandBuffer> mainThreadGraphicsCmdBufs;

    public:
        QueueSyncPool(uint16_t size);

        ~QueueSyncPool();
        Semaphore& GetSemaphore(VkSemaphore semaphore);


        void SemaphoreBeginSignaling(VkSemaphore semaphore) {
            GetSemaphore(semaphore).BeginSignaling();
        }
        void SemaphoreBeginWaiting(VkSemaphore semaphore) {
            GetSemaphore(semaphore).BeginWaiting();
        }
        void SemaphoreFinishedWaiting(VkSemaphore semaphore) {
            GetSemaphore(semaphore).FinishWaiting();
        }
        
        Semaphore* GetSemaphoreForSignaling();

        CommandBuffer& GetCmdBufSingleTime(Queue::Enum queue);
        bool CheckFencesForUsage();
        void CheckFencesForCallbacks();
        Fence& GetFence();
        GraphicsFence& GetMainThreadGraphicsFence();
    };
} //namespace EWE