#include "EightWinds/Backend/SubmitInfo.h"

#include "EightWinds/CommandBuffer.h"

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

        void SubmitInfo::ApplyWaitSemaphores(){
            internal_waitSemaphores.resize(waitSemaphores.size());
            for(std::size_t i = 0; i < waitSemaphores.size(); i++){
                memcpy(&internal_waitSemaphores[i], waitSemaphores[i], sizeof(VkSemaphoreSubmitInfo));
            }
        }

        VkSubmitInfo2 SubmitInfo::Expand(){
            EWE_ASSERT(commandInfos.size() == cmdBuffers.size());
            EWE_ASSERT(signalSemaphores.size() > 0 && "signaling with no context");
            ApplyWaitSemaphores();

            return VkSubmitInfo2{
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
                .pNext = nullptr,
                .waitSemaphoreInfoCount = static_cast<uint32_t>(internal_waitSemaphores.size()),
                .pWaitSemaphoreInfos = internal_waitSemaphores.data(),
                .commandBufferInfoCount = static_cast<uint32_t>(commandInfos.size()),
                .pCommandBufferInfos = commandInfos.data(),
                .signalSemaphoreInfoCount = static_cast<uint32_t>(signalSemaphores.size()),
                .pSignalSemaphoreInfos = signalSemaphores.data()
            };
        }

        VkSubmitInfo2 SubmitInfo::ExpandWithoutSignal(){
            EWE_ASSERT(commandInfos.size() == cmdBuffers.size());
            ApplyWaitSemaphores();
            return VkSubmitInfo2{
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
                .pNext = nullptr,
                .waitSemaphoreInfoCount = static_cast<uint32_t>(internal_waitSemaphores.size()),
                .pWaitSemaphoreInfos = internal_waitSemaphores.data(),
                .commandBufferInfoCount = static_cast<uint32_t>(commandInfos.size()),
                .pCommandBufferInfos = commandInfos.data(),
                .signalSemaphoreInfoCount = 0,
                .pSignalSemaphoreInfos = nullptr
            };
        }

        
        void SubmitInfo::WaitOnPrevious(SubmitInfo& previous){
            for(auto& prev_sigSem : previous.signalSemaphores){
                waitSemaphores.push_back(&prev_sigSem);
            }
        }
    } //namespace Backend
} //namespace EWE