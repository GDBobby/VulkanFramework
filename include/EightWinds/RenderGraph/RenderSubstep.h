#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/CommandBuffer.h"

#include "EightWinds/Buffer.h"
#include "EightWinds/Image.h"

#include <unordered_set>

//secondary command buffers
namespace EWE{


    template<typename T>
    struct ResourceUsage{
        T* resource
        bool writes; //read and write, or just write
    };


    struct RenderStage{
        LogicalDevice& device;

        //this could be locally owned commandbuffer, or externally referenced
        CommandBuffer* commandBuffer;

        std::string name;

        //potentially validate in debug mode to ensure there's no duplicates. not even sure if it matters
        std::vector<ResourceUsage<Buffer>> buffers;
        std::vector<ResourceUsage<Image>>  images;


        Queue& queue;
    };

    template<typename T>
    bool ResourceDependency(const std::vector<ResourceUsage<T>>& a, const std::vector<ResourceUsage<T>>& b) {
        for (auto& x : a) {
            for (auto& y : b) {
                if (x.resource == y.resource) {
                    uint32_t previousQueue = x.queueFamily;
                    uint32_t currentQueue = y.queueFamily;
                    uint32_t resourceQueue = x.resource->owningQueue;

                    bool queueDependency = (previousQueue != currentQueue ||
                                            previousQueue != resourceQueue);

                    if (x.writes || queueDependency)
                        return true;
                }
            }
        }
        return false;
    };

    bool hasDependency(const RenderStage& producer, const RenderStage& consumer) {


        return ResourceDependency(producer.buffers, consumer.buffers) ||
            ResourceDependency(producer.images, consumer.images);
    }
}