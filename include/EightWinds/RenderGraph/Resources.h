#pragma once

#include "EightWinds/VulkanHeader.h"

namespace EWE{
    struct Buffer;
    struct Image;
    

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
        PerFlight<T*> resource;
        UsageData<T> usage;

        [[nodiscard]] explicit Resource(UsageData<T> const& usage)
            :resource{nullptr},
            usage{usage}
        {}
        [[nodiscard]] explicit Resource(T& resource, UsageData<T> const& usage)
            : resource{ &resource },
            usage{ usage }
        {}
        [[nodiscard]] explicit Resource(PerFlight<T>& resource, UsageData<T> const& usage)
        :resource{&resource[0], &resource[1]},
            usage{ usage }
        {}
    };

    struct TaskResourceUsage{
        std::vector<Resource<Image>> images{};
        std::vector<Resource<Buffer>> buffers{};

        template<typename Res>
        uint32_t AddResource(UsageData<Res> const& usage) {
            if constexpr (std::is_same_v<Res, Buffer>) {
                buffers.push_back(
                    Resource<Buffer>{usage}
                );
                return buffers.size() - 1;
            }
            else if constexpr (std::is_same_v<Res, Image>) {
                images.push_back(
                    Resource<Image>{usage}
                );
                return images.size() - 1;
            }
            else {
                assert(false && "invalid resource type");
                //static_assert(false && "invalid resource type");
            }
            return 69420;
        }

        template<typename Res>
        uint32_t AddResource(Res& res, UsageData<Res> const& usage){
            if constexpr (std::is_same_v<Res, Buffer>){
                buffers.push_back(
                    Resource<Buffer>{res, usage}
                );
                return buffers.size() - 1;
            }
            else if constexpr (std::is_same_v<Res, Image>) {
                images.push_back(
                    Resource<Image>{res, usage}
                );
                return images.size() - 1;
            }
            else{
                assert(false && "invalid resource type");
                //static_assert(false && "invalid resource type");
            }
            return 69420;
        }
        template<typename Res>
        uint32_t AddResource(PerFlight<Res>& res, UsageData<Res> const& usage) {
            if constexpr (std::is_same_v<Res, PerFlight<Buffer>>) {
                buffers.push_back(
                    PerFlight<Resource<Buffer>>{
                        Resource<Buffer>{res, usage},
                    }
                );
                return buffers.size() - 1;
            }
            else if constexpr (std::is_same_v<Res, PerFlight<Image>>) {
                images.push_back(
                    PerFlight<Resource<Image>>{
                        Resource<Image>{res, usage}
                    }
                );
                return images.size() - 1;
            }
            else {
                assert(false && "invalid resource type");
                //static_assert(false && "invalid resource type");
            }
            return 69420;
        }
    };

    //transition and acquisition are created from something else, not directly constructed by the programmer
    template<typename Res>
    struct ResourceTransition{
        Resource<Res>* lhs;
        Resource<Res>* rhs;

        ResourceTransition(TaskResourceUsage& lhs, uint32_t lh_index, TaskResourceUsage& rhs, uint32_t rh_index)
        {
            if constexpr (std::is_same_v<Res, Buffer>) {
#if EWE_DEBUG_BOOL
                for (uint8_t i = 0; i < max_frames_in_flight; i++) {
                    assert(lhs.buffers[lh_index].resource[i] == rhs.buffers[rh_index].resource[i]);
                }
#endif 
                this->lhs = &lhs.buffers[lh_index];
                this->rhs = &rhs.buffers[rh_index];
            }
            else if constexpr (std::is_same_v<Res, Image>) {
#if EWE_DEBUG_BOOL
                for (uint8_t i = 0; i < max_frames_in_flight; i++) {
                    assert(lhs.images[lh_index].resource[i] == rhs.images[rh_index].resource[i]);
                }
#endif
                this->lhs = &lhs.images[lh_index];
                this->rhs = &rhs.images[rh_index];
            }
            else {
  //              static_assert(false);
            }
        }

        [[nodiscard]] ResourceTransition(ResourceTransition&& moveSrc) noexcept
        : lhs{moveSrc.lhs},
            rhs{moveSrc.rhs}
        {}

        ResourceTransition(ResourceTransition const& copySrc) = delete;
        ResourceTransition& operator=(ResourceTransition&& moveSrc) = delete;
        ResourceTransition& operator=(ResourceTransition const& copySrc) = delete;
    };

    //first time in use of frame, queue transfers aren't allowed here
    template<typename Res>
    struct ResourceAcquisition{
        Resource<Res>* rhs;

        ResourceAcquisition(TaskResourceUsage& rhs, uint32_t rh_index) {
            if constexpr (std::is_same_v<Res, Buffer>) {
                this->rhs = &rhs.buffers[rh_index];
            }
            else if constexpr (std::is_same_v<Res, Image>) {
                this->rhs = &rhs.images[rh_index];
            }
            else {
  //              static_assert(false);
            }
        }
    };


}