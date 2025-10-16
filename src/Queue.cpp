#include "EightWinds/Queue.h"

#include <stack>

namespace EWE {


    bool QueueFamily::SupportsGraphics() const {
        return (properties.queueFlags & vk::QueueFlagBits::eGraphics) == vk::QueueFlagBits::eGraphics;
    }
    bool QueueFamily::SupportsCompute() const {
        return (properties.queueFlags & vk::QueueFlagBits::eCompute) == vk::QueueFlagBits::eCompute;
    }
    bool QueueFamily::SupportsTransfer() const {
        return (properties.queueFlags & vk::QueueFlagBits::eTransfer) == vk::QueueFlagBits::eTransfer;
    }
    bool QueueFamily::SupportsSurfacePresent(vk::SurfaceKHR surface) const {
        return device.getSurfaceSupportKHR(index, surface);
    }

    QueueFamily::QueueFamily(vk::QueueFamilyProperties2 const& properties) 
    : properties{properties} 
    {}

    std::vector<QueueFamily> QueueFamily::Enumerate(vk::PhysicalDevice device) {
        auto queueFamilies = device.getQueueFamilyProperties2();
        
        std::vector<QueueFamily> queues(0);
        queues.reserve(queueFamilies.size());
        for (auto const& queueProps : queueFamilies) {
            queues.emplace_back(device, queueProps)
        }

    }



    std::vector<Queue> QueueRequest::RequestQueues(std::vector<QueueFamily> const& families, std::span<QueueRequest> requests) {
        /*
        std::vector<uint8_t> requestFlagCount(requests.size());
        for(uint8_t i = 0; i < Flags::Count; i++){
            requestFlagCount[i] = 0;
            #define FLAG_CONTAINS(a) requestFlagCount[i] = (a & requests[i]) == a
            FLAG_CONTAINS(Flags::eGraphics);
            FLAG_CONTAINS(Flags::eCompute);
            FLAG_CONTAINS(Flags::eTransfer);
            FLAG_CONTAINS(Flags::eSparseBinding);
            FLAG_CONTAINS(Flags::eProtected);
            FLAG_CONTAINS(Flags::eVideoDecodeKHR);
            FLAG_CONTAINS(Flags::eOpticalFlowNV);
            #undef FLAG_CONTAINS
        }

        std::array<std::vector<QueueFamily const*>, Flags::Count> availableQueues{};
        for(uint8_t i = 0; i < families.size(); i++){
            #define FLAG_PUSH_CONTAINS(a, b) if(families[i].properties.queueFlags & b){ \
                                                availableQueues[a].push_back(&families[i]);  }

            FLAG_CONTAINS(0, Flags::eGraphics);
            FLAG_CONTAINS(1, Flags::eCompute);
            FLAG_CONTAINS(2, Flags::eTransfer);
            FLAG_CONTAINS(3, Flags::eSparseBinding);
            FLAG_CONTAINS(4, Flags::eProtected);
            FLAG_CONTAINS(5, Flags::eVideoDecodeKHR);
            FLAG_CONTAINS(6, Flags::eOpticalFlowNV);

            //check surface here

            #undef FLAG_PUSH_CONTAINS
        }

        //if the queue is already in use. priority is based on the ordering in the span
        std::vector<bool> inUse(families.size(), false);

        std::vector<std::optional<Queue>> ret(requests.size(), std::nullopt);
        */
        

        //i want a designated graphics/present queue, or throw an error
        //i want a dedicated async compute queue
        //i want a dedicated async transfer queue
        //if i dont get two separate dedicated queues for transfer and compute, attempt to combine those 2
        //otherwise, flop them in the graphics queue

        bool foundDedicatedGraphicsPresent = false;
#ifdef EWE_DEBUG
        for (const auto& family : families) {
            printf("queue properties - %d:%d:%d\n", family.queueFlags & VK_QUEUE_GRAPHICS_BIT, family.queueFlags & VK_QUEUE_COMPUTE_BIT, family.queueFlags & VK_QUEUE_TRANSFER_BIT);

        }
#endif
        //fidning graphics/present queue
        int currentIndex = 0; 
        for (const auto& queueFamily : queueFamilies) {
            VkBool32 presentSupport = false;
            EWE_VK(vkGetPhysicalDeviceSurfaceSupportKHR, device, currentIndex, surface, &presentSupport);
            printf("queue present support[%d] : %d\n", currentIndex, presentSupport);
            bool graphicsSupport = (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) == vk::QueueFlagBits::eGraphics;
            bool computeSupport = queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT;
            const bool computeSupport = queueFamily.queueFlags & vk::QueueFlagBits::eCompute;
            if ((presentSupport && graphicsSupport && computeSupport) == true) {
                foundDedicatedGraphicsPresent = true;
                VK::Object->queueIndex[Queue::graphics] = currentIndex;
                VK::Object->queueIndex[Queue::present] = currentIndex;
                found[Queue::graphics] = true;
                found[Queue::present] = true;
                break;
            }
            currentIndex++;
        }
        assert(foundDedicatedGraphicsPresent && "failed to find a graphics/present queue that could also do compute");

        //re-searching for compute and transfer queues
        std::stack<int> dedicatedComputeFamilies{};
        std::stack<int> dedicatedTransferFamilies{};
        std::stack<int> combinedTransferComputeFamilies{};

        currentIndex = 0;
        for (const auto& family : families) {
            if (currentIndex == VK::Object->queueIndex[Queue::graphics]) {
                currentIndex++;
                continue;
            }
            bool computeSupport = family.queueFlags & VK_QUEUE_COMPUTE_BIT;
            bool transferSupport = family.queueFlags & VK_QUEUE_TRANSFER_BIT;
            printf("queue support[%d] - %d:%d \n", currentIndex, computeSupport, transferSupport);
            if (computeSupport && transferSupport) {
                combinedTransferComputeFamilies.push(currentIndex);
            }
            else if (computeSupport) {
                dedicatedComputeFamilies.push(currentIndex);
            }
            else if (transferSupport) {
                dedicatedTransferFamilies.push(currentIndex);
            }
            currentIndex++;
        }
        printf("after the queue family \n");
        if (dedicatedComputeFamilies.size() > 0) {
            VK::Object->queueIndex[Queue::compute] = dedicatedComputeFamilies.top();
            found[Queue::compute] = true;
        }
        if (dedicatedTransferFamilies.size() > 0) {
            VK::Object->queueIndex[Queue::transfer] = dedicatedTransferFamilies.top();
            found[Queue::transfer] = true;
        }
        if (combinedTransferComputeFamilies.size() > 0) {
            if ((!found[Queue::compute]) && (!found[Queue::transfer])) {
                //assert(combinedTransferComputeFamilies.size() >= 2 && "not enough queues for transfer and compute");

                //VK::Object->queueIndex[Queue::compute] = combinedTransferComputeFamilies.top();
                //found[Queue::compute] = true;
                //combinedTransferComputeFamilies.pop();

                //need a flag in VK::Object for combined transfer and compute


                VK::Object->queueIndex[Queue::transfer] = combinedTransferComputeFamilies.top();
                found[Queue::transfer] = true;
            }
            else if (!found[Queue::compute]) {
                VK::Object->queueIndex[Queue::compute] = combinedTransferComputeFamilies.top();
                found[Queue::compute] = true;
            }
            else if (!found[Queue::transfer]) {
                VK::Object->queueIndex[Queue::transfer] = combinedTransferComputeFamilies.top();
                found[Queue::transfer] = true;
            }
        }
        //assert(found[Queue::compute] && found[Queue::transfer] && "did not find a dedicated transfer or compute queue");
        //assert(VK::Object->queueIndex[Queue::compute] != VK::Object->queueIndex[Queue::transfer] && "compute queue and transfer q should not be the same");

        if (!found[Queue::compute]) {
            printf("missing dedicated compute queue, potentially fatal crash incoming if this hasn't been resolved yet (if you don't crash it has)\n");
        }
        for (uint8_t i = 0; i < Queue::_count; i++) {
            VK::Object->queueEnabled[i] = found[i];
        }

        return found[Queue::graphics] && found[Queue::present];// && found[Queue::transfer];//&& found[Queue::compute];
    }


}