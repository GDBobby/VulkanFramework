#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/RenderGraph/GPUTask.h"

#include "EightWinds/PerFlight.h"

namespace EWE{
    struct Buffer;
    struct Image;
    
    template<typename T>
    struct Resource{};

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

    template<>
    struct Resource<Buffer> {
        Buffer* buffer;
        UsageData<Buffer> usage;
    };
    template<>
    struct Resource<Image> {
        Image* image;
        UsageData<Image> usage;
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

    template<typename T>
    struct BarrierResource {
        T* resource;
    };
    template<>
    struct BarrierResource<Buffer> {
        Buffer* resource;//the ownign queue might change
    };
    template<>
    struct BarrierResource<Image> {
        Image* resource; //need to change the layout
        VkImageLayout finalLayout;
    };

    template<typename Res>
    struct ResourceTransition{
        Resource<Res>* lhs;
        Resource<Res>* rhs;
    };

    //first time in use of frame
    template<typename Res>
    struct ResourceAcquisition{
        Resource<Res>* rhs;

        //pass in the same queue if it's not a queue transfer
        //this will also compare the layout 
        void Acquire(Queue& lhsQueue); 
    };

    struct TaskPrefix{
        Queue& queue;

        std::vector<ResourceTransition<Image>> imageTransitions;
        std::vector<ResourceAcquisition<Image>> imageAcquisitions;

        std::vector<PerFlight<ResourceTransition<Image>>> imageTransitions_perFlight;
        std::vector<PerFlight<ResourceAcquisition<Image>>> imageAcquisitions_perFlight;

        std::vector<ResourceTransition<Buffer>> bufferTransitions;
        std::vector<ResourceAcquisition<Buffer>> bufferAcquisitions;

        std::vector<PerFlight<ResourceTransition<Buffer>>> bufferTransitions_perFlight;
        std::vector<PerFlight<ResourceAcquisition<Buffer>>> bufferAcquisitions_perFlight;

        void Execute(CommandBuffer& cmdBuf, uint8_t frameIndex);
    };

    //these suffixes are ONLY for queue transfers
    //if it's not a queue transfer, it ONLY needs to be put in the prefix
    //the suffix transition NEEDS to PERFECTLY MATCH the partner prefix transition
    struct TaskSuffix{
        Queue& queue;

        std::vector<ResourceTransition<Image>> imageTransitions;
        std::vector<ResourceTransition<Buffer>> bufferTransitions;
        
        std::vector<PerFlight<ResourceTransition<Image>>> imageTransitions_perFlight;
        std::vector<PerFlight<ResourceTransition<Buffer>>> bufferTransitions_perFlight;

        void Execute(CommandBufer& cmdBuf, uint8_t frameIndex);
    };

    /*
    so we can go prefix -> prefix
    OR we can go suffix -> prefix
    but we can't go prefix->suffix
    */

    //do I do it like this or do I put them directly in the task?
    //the main thign is that sometimes an affix will be requested without a task
    struct TaskAffixes{
        TaskPrefix prefix;
        TaskSuffix suffix;
    };
}