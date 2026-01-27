#pragma once

#include "EightWinds/VulkanHeader.h"

namespace EWE{
	
	struct Buffer;
	struct Image;
	struct GPUTask;
	
    struct SynchronizationManager {
        template<typename Resource>
        struct TransitionObjects {
            GPUTask* lhs;
            uint32_t lh_index;
            GPUTask* rhs;
            uint32_t rh_index;
        };
        template<typename Resource>
        struct AcquireObjects{
            GPUTask* rhs;
            uint32_t rh_index;
        };

        std::vector<TransitionObjects<Buffer>> buffer_transitions;
        std::vector<TransitionObjects<Image>> image_transitions;
        std::vector<AcquireObjects<Buffer>> buffer_acquisitions;
        std::vector<AcquireObjects<Image>> image_acquisitions;

        void AddTransition_Buffer(GPUTask& lhs, uint32_t lh_index, GPUTask& rhs, uint32_t rh_index);
        void AddTransition_Image(GPUTask& lhs, uint32_t lh_index, GPUTask& rhs, uint32_t rh_index);
        void AddAcquisition_Buffer(GPUTask& rhs, uint32_t rh_index);
        void AddAcquisition_Image(GPUTask& rhs, uint32_t rh_index);

        void PopulateAffixes(uint8_t frameIndex);

    };
}