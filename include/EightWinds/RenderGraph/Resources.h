#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/Data/PerFlight.h"


namespace EWE{
    struct Buffer;
    struct Image;
    

    template<typename T>
    struct UsageData;
    //https://vulkan.lunarg.com/doc/view/1.4.328.1/windows/antora/spec/latest/chapters/synchronization.html#synchronization-access-types-supported
    //some accessmask are only allowed within some pipeline stages, use the above link ^ to reference
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

        template<class>
        static constexpr bool invalid_type_debugging_helper = false;

        template<typename Res>
        uint32_t AddResource(UsageData<Res> const& usage) {
            if constexpr (std::is_same_v<Res, Buffer>) {
                buffers.emplace_back(usage);
                return buffers.size() - 1;
            }
            else if constexpr (std::is_same_v<Res, Image>) {
                images.emplace_back(usage);
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
                buffers.emplace_back(res, usage);
                return buffers.size() - 1;
            }
            else if constexpr (std::is_same_v<Res, Image>) {
                images.emplace_back(res, usage);
                return images.size() - 1;
            }
            else{
                //assert(invalid_type_debugging_helper<Res> && "invalid resource type");
                static_assert(invalid_type_debugging_helper<Res> && "invalid resource type");
            }
            return 69420;
        }
        template<typename Res>
        uint32_t AddResource(PerFlight<Res>& res, UsageData<Res> const& usage) {
            if constexpr (std::is_same_v<Res, Buffer>) {
                buffers.emplace_back(res, usage);
                return buffers.size() - 1;
            }
            else if constexpr (std::is_same_v<Res, Image>) {
                images.emplace_back(res, usage);
                return images.size() - 1;
            }
            else {
                //assert(invalid_type_debugging_helper<Res> && "invalid resource type");
                static_assert(invalid_type_debugging_helper<Res> && "invalid resource type");
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