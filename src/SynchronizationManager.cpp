#include "EightWinds/RenderGraph/SynchronizationManager.h"

#include "EightWinds/RenderGraph/GPUTask.h"
#include "EightWinds/Image.h"
#include "EightWinds/Buffer.h"

#include "EightWinds/Backend/PipelineBarrier.h"


namespace EWE{
    void SynchronizationManager::AddTransition_Buffer(GPUTask& lhs, uint32_t lh_index, GPUTask& rhs, uint32_t rh_index) {
        buffer_transitions.emplace_back(
            TransitionObjects<Buffer>{
                .lhs = &lhs,
                .lh_index = lh_index,
                .rhs = &rhs,
                .rh_index = rh_index
            }
        );
    }

    void SynchronizationManager::AddTransition_Image(GPUTask& lhs, uint32_t lh_index, GPUTask& rhs, uint32_t rh_index) {
        image_transitions.emplace_back(
            TransitionObjects<Image>{
                .lhs = &lhs,
                .lh_index = lh_index,
                .rhs = &rhs,
                .rh_index = rh_index
            }
        );
    }
    void SynchronizationManager::AddAcquisition_Buffer(GPUTask& rhs, uint32_t rh_index) {
        buffer_acquisitions.emplace_back(
            AcquireObjects<Buffer>{
                .rhs = &rhs,
                .rh_index = rh_index
            }
        );
    }
    void SynchronizationManager::AddAcquisition_Image(GPUTask& rhs, uint32_t rh_index) {
        image_acquisitions.emplace_back(
            AcquireObjects<Image>{
                .rhs = &rhs,
                .rh_index = rh_index
            }s
        );
    }

    void SynchronizationManager::PopulateAffixes(uint8_t frameIndex){

		//all barriers need to have been cleared before populating, 
		// potentially put validaiton in here that all touched tasks have 0 existing barriers
		for (auto& trans : buffer_transitions) {
			auto const& barr = Barrier::Transition_Buffer(trans.lhs.queue, trans.lh_index, *trans.rhs, trans.rh_index, frameIndex);
			if (trans.lhs->queue != trans.rhs->queue) {
				//put a suffix on lhs
				auto& lh_barriers = trans.lhs->suffix.barriers[frameIndex];
				lh_barriers.bufferBarriers.push_back(barr);
			}
			auto& rh_barriers = trans.rhs->prefix.barriers[frameIndex];
			rh_barriers.bufferBarriers.push_back(barr);
		}
		for (auto& acq : buffer_acquisitions) {
			auto& rh_barriers = acq.rhs->prefix.barriers[frameIndex];
			rh_barriers.bufferBarriers.push_back(Acquire_Buffer(*acq.rhs, acq.rh_index, frameIndex));
		}

		for (auto& trans : image_transitions) {
			auto const& barr = Transition_Image(*trans.lhs, trans.lh_index, *trans.rhs, trans.rh_index, frameIndex);
			if (trans.lhs->queue != trans.rhs->queue) {
				//put a suffix on lhs
				auto& lh_barriers = trans.lhs->suffix.barriers[frameIndex];
				trans.lhs->suffix.barriers[frameIndex].imageBarriers.push_back(barr);
			}
			auto& rh_res = trans.rhs->resources.images[trans.rh_index];
			trans.rhs->prefix.image_updates.emplace_back(trans.rhs->resources.images[trans.rh_index].resource[frameIndex], trans.rhs->resources.images[trans.rh_index].usage.layout);

			auto& rh_barriers = trans.rhs->prefix.barriers[frameIndex];
			rh_barriers.imageBarriers.push_back(barr);

		}
		for (auto& acq : image_acquisitions) {
			auto& rh_res = acq.rhs->resources.images[acq.rh_index];
			acq.rhs->prefix.image_updates.emplace_back(acq.rhs->resources.images[acq.rh_index].resource[frameIndex], acq.rhs->resources.images[acq.rh_index].usage.layout);

			auto& rh_barriers = acq.rhs->prefix.barriers[frameIndex];
			rh_barriers.imageBarriers.push_back(Acquire_Image(*acq.rhs, acq.rh_index, frameIndex));
		}
		
    }
}