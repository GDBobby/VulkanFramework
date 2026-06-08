#include "EightWinds/Backend/GarbageDisposal.h"

#include "EightWinds/LogicalDevice.h"

namespace EWE{
namespace Backend{

    GarbageItem::GarbageItem(GarbageItem&& moveSrc)
    : tossedDuration{moveSrc.tossedDuration},
        Destroy{moveSrc.Destroy}
    {
        moveSrc.Destroy = nullptr;
    }

    GarbageItem& GarbageItem::operator=(GarbageItem&& moveSrc){
        tossedDuration = moveSrc.tossedDuration;
        Destroy = moveSrc.Destroy;
        moveSrc.Destroy = nullptr;
        return *this;
    }

    GarbageDisposal::GarbageDisposal(LogicalDevice& logicalDevice)
        : device{ logicalDevice }
    {
    }
    GarbageDisposal::~GarbageDisposal(){
        Clear();
    }

    void GarbageDisposal::Clear(){
        //device wait idle?
        items.clear();
    }

    void GarbageDisposal::Tick(){
        for (auto iter = items.begin(); iter != items.end();) {
            if(iter->tossedDuration++ > max_frames_in_flight){
                iter = items.erase(iter);
                continue;
            }
            iter++;
        }
    }

    void GarbageDisposal::Toss(std::function<void()> destroyer){
        item_mut.lock();
        GarbageItem& backRef = items.emplace_back();
        backRef.Destroy = destroyer;
        item_mut.unlock();
    }

    template<> void GarbageDisposal::TossVK<VkSemaphore>(VkSemaphore sem){
        Toss(
            [&logicalDevice = device, sem](){
                vkDestroySemaphore(logicalDevice, sem, nullptr);
            }
        );
    }
    template<> void GarbageDisposal::TossVK<VkFence>(VkFence fence){
        Toss(
            [&logicalDevice = device, fence](){
                vkDestroyFence(logicalDevice, fence, nullptr);
            }
        );
    }
    template<> void GarbageDisposal::TossVK<VkBuffer>(VkBuffer buffer){
        Toss(
            [&logicalDevice = device, buffer](){
                vkDestroyBuffer(logicalDevice, buffer, nullptr);
            }
        );
    }
    template<> void GarbageDisposal::TossVK<VkSampler>(VkSampler sampler){
        Toss(
            [&logicalDevice = device, sampler](){
                vkDestroySampler(logicalDevice, sampler, nullptr);
            }
        );
    }
}//namespace Backend
} //namespace EWE