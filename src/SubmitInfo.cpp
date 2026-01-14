#include "EightWinds/Backend/SubmitInfo.h"

#include "EightWinds/CommandBuffer.h"

#include <iterator>

namespace EWE{
    namespace Backend{
        void SubmitInfo::AddCommandBuffer(CommandBuffer& cmdBuf){
            cmdBuffers.push_back(&cmdBuf);
            commandInfos.emplace_back(
                VkCommandBufferSubmitInfo{
                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
                    .pNext = nullptr,
                    .commandBuffer = cmdBuf,
                    .deviceMask = 0
                }
            );
        }

        VkSubmitInfo2 SubmitInfo::Expand(){
    #if EWE_DEBUG_BOOL
            assert(commandInfos.size() == cmdBuffers.size());
            assert(signalSemaphores.size() > 0 && "signaling with no context");
    #endif
            return VkSubmitInfo2{
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
                .pNext = nullptr,
                .waitSemaphoreInfoCount = static_cast<uint32_t>(waitSemaphores.size()),
                .pWaitSemaphoreInfos = waitSemaphores.data(),
                .commandBufferInfoCount = static_cast<uint32_t>(commandInfos.size()),
                .pCommandBufferInfos = commandInfos.data(),
                .signalSemaphoreInfoCount = static_cast<uint32_t>(signalSemaphores.size()),
                .pSignalSemaphoreInfos = signalSemaphores.data()
            };
        }

        VkSubmitInfo2 SubmitInfo::ExpandWithoutSignal(){
#if EWE_DEBUG_BOOL
            assert(commandInfos.size() == cmdBuffers.size());
#endif
            return VkSubmitInfo2{
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
                .pNext = nullptr,
                .waitSemaphoreInfoCount = static_cast<uint32_t>(waitSemaphores.size()),
                .pWaitSemaphoreInfos = waitSemaphores.data(),
                .commandBufferInfoCount = static_cast<uint32_t>(commandInfos.size()),
                .pCommandBufferInfos = commandInfos.data(),
                .signalSemaphoreInfoCount = 0,
                .pSignalSemaphoreInfos = nullptr
            };
        }

        
        void SubmitInfo::WaitOnPrevious(SubmitInfo& previous){
            std::copy(previous.signalSemaphores.begin(), previous.signalSemaphores.end(), std::back_inserter(waitSemaphores));
        }
    } //namespace Backend
} //namespace EWE