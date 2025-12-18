#include "EightWinds/Backend/GarbageDisposal.h"

namespace EWE{
    namespace Backend{

        //i could just put this in the deconstructor
        void GarbageDisposal::GarbageItem::Destroy(VkDevice const vkDevice){
            switch(objectType){
                case VK_OBJECT_TYPE_SEMAPHORE: vkDestroySemaphore(vkDevice, reinterpret_cast<VkSemaphore>(object), nullptr);
                case VK_OBJECT_TYPE_FENCE: vkDestroyFence(vkDevice, reinterpret_cast<VkFence>(object), nullptr);
                case VK_OBJECT_TYPE_BUFFER: vkDestroyBuffer(vkDevice, reinterpret_cast<VkBuffer>(object), nullptr);
                case VK_OBJECT_TYPE_SAMPLER: vkDestroySampler(vkDevice, reinterpret_cast<VkSampler>(object), nullptr);
                default: EWE_UNREACHABLE;
            }
        }

        GarbageDisposal::GarbageDisposal(VkDevice device)
            : vkDevice{ device }
        {
        }
        GarbageDisposal::~GarbageDisposal(){
            Clear();
        }

        void GarbageDisposal::Clear(){
            for(auto& item : items){
                item.Destroy(vkDevice);
            }
            items.clear();
        }

        void GarbageDisposal::Tick(){
            for (auto iter = items.begin(); iter != items.end();) {
                if(iter->tossedDuration++ > max_frames_in_flight){
                    iter->Destroy(vkDevice);
                    iter = items.erase(iter);
                    continue;
                }
                iter++;
            }
        }

        void GarbageDisposal::Toss(void* object, VkObjectType objectType){
            items.emplace_back(object, objectType);
        }
    }//namespace Backend
} //namespace EWE