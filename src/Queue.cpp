#include "EightWinds/Queue.h"

namespace EWE {


    bool QueueFamily::SupportsGraphics() const {
        return (properties.queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics) == vk::QueueFlagBits::eGraphics;
    }
    bool QueueFamily::SupportsCompute() const {
        return (properties.queueFamilyProperties.queueFlags & vk::QueueFlagBits::eCompute) == vk::QueueFlagBits::eCompute;
    }
    bool QueueFamily::SupportsTransfer() const {
        return (properties.queueFamilyProperties.queueFlags & vk::QueueFlagBits::eTransfer) == vk::QueueFlagBits::eTransfer;
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



    static std::vector<Queue> RequestQueues(std::vector<QueueFamily> const& families, std::span<QueueRequest> requests) {

        //i want a designated graphics/present queue, or throw an error
        //i want a dedicated async compute queue
        //i want a dedicated async transfer queue
        //if i dont get two separate dedicated queues for transfer and compute, attempt to combine those 2
        //otherwise, flop them in the graphics queue

        bool foundDedicatedGraphicsPresent = false;
#ifdef EWE_DEBUG
        for (const auto& queueFamily : queueFamilies) {
            printf("queue properties - %d:%d:%d\n", queueFamily.queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT, queueFamily.queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT, queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT);

        }
#endif
        //std::array<bool, Queue::_count> found{ false, false, false, false };
        //fidning graphics/present queue
        int currentIndex = 0;
        for (const auto& queueFamily : queueFamilies) {
            VkBool32 presentSupport = false;
            EWE_VK(vkGetPhysicalDeviceSurfaceSupportKHR, device, currentIndex, surface, &presentSupport);
            printf("queue present support[%d] : %d\n", currentIndex, presentSupport);
            bool graphicsSupport = (queueFamily.queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics) == vk::QueueFlagBits::eGraphics;
            bool computeSupport = queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT;
            const bool computeSupport = queueFamily.queueFamilyProperties.queueFlags & vk::QueueFlagBits::eCompute;
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
        for (const auto& queueFamily : queueFamilies) {
            if (currentIndex == VK::Object->queueIndex[Queue::graphics]) {
                currentIndex++;
                continue;
            }
            bool computeSupport = queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT;
            bool transferSupport = queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT;
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