#include "EightWinds/RenderGraph/SynchronizationManager.h"

#include "EightWinds/RenderGraph/GPUTask.h"
#include "EightWinds/Image.h"
#include "EightWinds/Buffer.h"

#include "EightWinds/Backend/PipelineBarrier.h"


namespace EWE{

    template<> void SynchronizationManager::AddTransition<Buffer>(TaskResourceIndex const& lh, TaskResourceIndex const& rh) {
        buffer_transitions.emplace_back(
            TransitionObjects{
                .lh{lh},
                .rh{rh}
            }
        );
    }

    template<> void SynchronizationManager::AddTransition<Image>(TaskResourceIndex const& lh, TaskResourceIndex const& rh) {
        image_transitions.emplace_back(
            TransitionObjects{
                .lh{lh},
                .rh{rh}
            }
        );
    }
    template<> void SynchronizationManager::AddAcquisition<Buffer>(TaskResourceIndex const& rh, AcquireType acqType) {
        buffer_acquisitions.emplace_back(AcquireObject{.task = rh.task, .index = rh.index, .acqType = acqType });
    }
    template<> void SynchronizationManager::AddAcquisition<Image>(TaskResourceIndex const& rh, AcquireType acqType) {
        image_acquisitions.emplace_back(AcquireObject{.task = rh.task, .index = rh.index, .acqType = acqType });
    }


    template<> void SynchronizationManager::AddTransition<Buffer>(GPUTask& lhs, uint32_t lh_index, GPUTask& rhs, uint32_t rh_index) {
        buffer_transitions.emplace_back(
            TransitionObjects{
                .lh{
                    .task = &lhs,
                    .index = lh_index
                },
                .rh{
                    .task = &rhs,
                    .index = rh_index
                }
            }
        );
    }

    template<> void SynchronizationManager::AddTransition<Image>(GPUTask& lhs, uint32_t lh_index, GPUTask& rhs, uint32_t rh_index) {
        image_transitions.emplace_back(
            TransitionObjects{
                .lh{
                    .task = &lhs,
                    .index = lh_index
                },
                .rh{
                    .task = &rhs,
                    .index = rh_index
                }
            }
        );
    }
    template<> void SynchronizationManager::AddAcquisition<Buffer>(GPUTask& rhs, uint32_t index, AcquireType acqType) {
        buffer_acquisitions.emplace_back(
            AcquireObject{.task = &rhs, .index = index, .acqType = AcquireType::None }
        );
    }
    template<> void SynchronizationManager::AddAcquisition<Image>(GPUTask& rhs, uint32_t index, AcquireType acqType) {
        image_acquisitions.emplace_back(
            AcquireObject{.task = &rhs, .index = index, .acqType = acqType }
        );
    }

    void SynchronizationManager::PopulateAffixes(uint8_t frameIndex){

		//all barriers need to have been cleared before populating, 
		// potentially put validaiton in here that all touched tasks have 0 existing barriers
		for (auto& trans : buffer_transitions) {
			auto const& barr = Barrier::Transition<Buffer>(
                trans.lh.task->queue, *trans.lh.task->resources.buffers[trans.lh.index].resource[frameIndex], 
                trans.rh.task->queue, 
                trans.lh.task->resources.buffers[trans.lh.index].usage,
                trans.rh.task->resources.buffers[trans.rh.index].usage
            );
			if (trans.lh.task->queue != trans.rh.task->queue) {
				//put a suffix on lhs
				auto& lh_barriers = trans.lh.task->suffix.barriers[frameIndex];
				lh_barriers.bufferBarriers.push_back(barr);
			}
			auto& rh_barriers = trans.rh.task->prefix.barriers[frameIndex];
			rh_barriers.bufferBarriers.push_back(barr);
		}
		for (auto& acq : buffer_acquisitions) {
			auto& rh_barriers = acq.task->prefix.barriers[frameIndex];
			rh_barriers.bufferBarriers.push_back(
                Barrier::Acquire<Buffer>(
                    acq.task->queue, 
                    *acq.task->resources.buffers[acq.index].resource[frameIndex], 
                    acq.task->resources.buffers[acq.index].usage
                )
            );
		}

		for (auto& trans : image_transitions) {
			auto const& barr = Barrier::Transition<Image>(
                trans.lh.task->queue, 
                *trans.lh.task->resources.images[trans.lh.index].resource[frameIndex], 
                trans.rh.task->queue,
                trans.lh.task->resources.images[trans.lh.index].usage, 
                trans.rh.task->resources.images[trans.rh.index].usage
            );
			if (trans.lh.task->queue != trans.rh.task->queue) {
				//put a suffix on lhs
				auto& lh_barriers = trans.lh.task->suffix.barriers[frameIndex];
				trans.lh.task->suffix.barriers[frameIndex].imageBarriers.push_back(barr);
			}
			auto& rh_res = trans.rh.task->resources.images[trans.rh.index];
			trans.rh.task->prefix.image_updates.emplace_back(
                trans.rh.task->resources.images[trans.rh.index].resource[frameIndex], 
                trans.rh.task->resources.images[trans.rh.index].usage.layout
            );

			auto& rh_barriers = trans.rh.task->prefix.barriers[frameIndex];
			rh_barriers.imageBarriers.push_back(barr);

		}
		for (auto& acq : image_acquisitions) {
			auto& rh_res = acq.task->resources.images[acq.index];
			acq.task->prefix.image_updates.emplace_back(
                acq.task->resources.images[acq.index].resource[frameIndex], 
                acq.task->resources.images[acq.index].usage.layout
            );

			auto& rh_barriers = acq.task->prefix.barriers[frameIndex];
			rh_barriers.imageBarriers.push_back(
                Barrier::Acquire<Image>(
                    acq.task->queue, 
                    *acq.task->resources.images[acq.index].resource[frameIndex], 
                    acq.task->resources.images[acq.index].usage,
                    acq.acqType
                )
            );
		}
		
    }
}