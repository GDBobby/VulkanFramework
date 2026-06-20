#pragma once

#include "EightWinds/VulkanHeader.h"

#include "EightWinds/RenderGraph/Resources.h"

#include <cstdint>
#include <vector>

namespace EWE{
	
	struct Buffer;
	struct Image;
	struct GPUTask;

    struct TaskResourceIndex{
        GPUTask* task;
        uint32_t index;
    };

	
    struct SynchronizationManager {
        struct TransitionObjects {
            TaskResourceIndex lh;
            TaskResourceIndex rh;
        };
        struct AcquireObject {
            GPUTask* task;
            uint32_t index;
            AcquireType acqType;
        };

        std::vector<TransitionObjects> buffer_transitions;
        std::vector<TransitionObjects> image_transitions;
        std::vector<AcquireObject> buffer_acquisitions;
        std::vector<AcquireObject> image_acquisitions;



        template<ResourceType Resource>
        void AddTransition(TaskResourceIndex const& lh, TaskResourceIndex const& rh);
        template<ResourceType Resource>
        void AddAcquisition(TaskResourceIndex const& rh, AcquireType acqType = AcquireType::None);

        template<ResourceType Resource>
        void AddTransition(GPUTask& lhs, uint32_t lh_index, GPUTask& rhs, uint32_t rh_index);
        template<ResourceType Resource>
        void AddAcquisition(GPUTask& rhs, uint32_t rh_index, AcquireType acqType = AcquireType::None);

        void PopulateAffixes(uint8_t frameIndex);

    };


        template<> void SynchronizationManager::AddTransition<Buffer>(TaskResourceIndex const& lh, TaskResourceIndex const& rh);
        template<> void SynchronizationManager::AddAcquisition<Buffer>(TaskResourceIndex const& rh, AcquireType acqType);
        template<> void SynchronizationManager::AddTransition<Buffer>(GPUTask& lhs, uint32_t lh_index, GPUTask& rhs, uint32_t rh_index);
        template<> void SynchronizationManager::AddAcquisition<Buffer>(GPUTask& rhs, uint32_t rh_index, AcquireType acqType);
        
        template<> void SynchronizationManager::AddTransition<Image>(TaskResourceIndex const& lh, TaskResourceIndex const& rh);
        template<> void SynchronizationManager::AddAcquisition<Image>(TaskResourceIndex const& rh, AcquireType acqType);
        template<> void SynchronizationManager::AddTransition<Image>(GPUTask& lhs, uint32_t lh_index, GPUTask& rhs, uint32_t rh_index);
        template<> void SynchronizationManager::AddAcquisition<Image>(GPUTask& rhs, uint32_t rh_index, AcquireType acqType);
}