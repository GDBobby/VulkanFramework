#pragma once

#include "EightWinds/VulkanHeader.h"

namespace EWE{
    struct Buffer;
    struct Image;

    struct Queue;
    

    template<typename T>
    struct UsageData;

    template<>
    struct UsageData<Buffer> {
        VkPipelineStageFlags2 stage;
        VkAccessFlags2 accessMask;
    };
    template<>
    struct UsageData<Image>{
        VkPipelineStageFlags2 stage;
        VkAccessFlags2 accessMask;
        VkImageLayout layout;
    };

    template<typename T>
    struct Resource {
        T* resource;
        UsageData<T> usage;
    };

    struct TaskResourceUsage{
        Queue& queue;

        std::vector<Resource<Image>> images;
        std::vector<Resource<Buffer>> buffers;
        
        std::vector<PerFlight<Resource<Image>>> images_perFlight;
        std::vector<PerFlight<Resource<Buffer>>> buffers_perFlight;

        template<typename Res>
        void AddResource(Res& res, UsageData<Res> const& usage){
            if constexpr (std::is_same_v<Res, Buffer>){
                buffers.push_back(Resource<Buffer>{.buffer = &res, .usage = usage});
            }
            else if constexpr (std::is_same_v<Res, Image>){
                images.push_back(Resource<Image>{.image = &res, .usage = usage});
            }
            else if constexpr (std::is_same_v<Res, PerFlight<Buffer>>){
                buffers_perFlight.push_back(
                    PerFlight<Resource<Buffer>>{
                        Resource<Buffer>{.buffer = &res[0], .usage = usage},
                        Resource<Buffer>{.buffer = &res[1], .usage = usage}
                    }
                );
            }
            else if constexpr (std::is_same_v<Res, PerFlight<Image>>){
                images_perFlight.push_back(
                    PerFlight<Resource<Image>>{
                        Resource<Image>{.image = &res[0], .usage = usage},
                        Resource<Image>{.image = &res[1], .usage = usage}
                    }
                );
            }
            else{
                static_assert(false && "invalid resource type");
            }
        }

    };

    template<typename Res>
    struct ResourceTransition{
        PerFlight<Resource<Res>*> lhs;
        PerFlight<Resource<Res>*> rhs;
    };

    //first time in use of frame
    template<typename Res>
    struct ResourceAcquisition{
        PerFlight<Resource<Res>> rhs; //its fine to make a copy of Resource and UsageData, they're light weight

        ResourceAcquisition(PerFlight<Res>& base_resource, UsageData<Res> const& usage) {
            for (uint8_t i = 0; i < max_frames_in_flight; i++) {
                rhs[i].resource = &base_resource[i];
                rhs[i].usage = usage;
            }
        }

        //pass in the same queue if it's not a queue transfer
        //this will also compare the layout 
        void Acquire(Queue& lhsQueue); 
    };

    struct TaskPrefix{
        LogicalDevice& logicalDevice;
        Queue& queue;
        [[nodiscard]] explicit TaskPrefix(LogicalDevice& logicalDevice, Queue& queue) 
            : logicalDevice{ logicalDevice }, queue{ queue } 
        {}

        std::vector<PerFlight<ResourceTransition<Image>>> imageTransitions;
        std::vector<PerFlight<ResourceAcquisition<Image>>> imageAcquisitions;

        std::vector<PerFlight<ResourceTransition<Buffer>>> bufferTransitions;
        std::vector<PerFlight<ResourceAcquisition<Buffer>>> bufferAcquisitions;

        inline bool Empty() const noexcept {
            return (imageTransitions.size() + imageAcquisitions.size() + bufferTransitions.size() + bufferAcquisitions.size()) == 0;
        }

        void Execute(CommandBuffer& cmdBuf, uint8_t frameIndex);
    };

    //these suffixes are ONLY for queue transfers
    //if it's not a queue transfer, it ONLY needs to be put in the prefix
    //the suffix transition NEEDS to PERFECTLY MATCH the partner prefix transition
    struct TaskSuffix{
        LogicalDevice& logicalDevice;
        Queue& queue;
        [[nodiscard]] explicit TaskSuffix(LogicalDevice& logicalDevice, Queue& queue)
            : logicalDevice{ logicalDevice }, queue{ queue }
        {}

        std::vector<ResourceTransition<Image>> imageTransitions;
        std::vector<ResourceTransition<Buffer>> bufferTransitions;

        inline bool Empty() const noexcept {
            return (imageTransitions.size() + bufferTransitions.size()) == 0;
        }

        void Execute(CommandBuffer& cmdBuf, uint8_t frameIndex);
    };
}