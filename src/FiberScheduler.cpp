#include "EightWinds/Data/Fiber/Scheduler.h"

namespace EWE{
	namespace Fiber{
			
		thread_local Context* Scheduler::current = nullptr;

		Scheduler::Scheduler(std::size_t threads, uint8_t fibersPerThread) 
		: pool(threads * fibersPerThread) 
		{
			for (std::size_t i = 0; i < threads; i++)
				workers.emplace_back(
					[this]{ 
						WorkerLoop(); 
					}
				);
		}

		void Scheduler::Spawn(std::function<void()> func) {
			auto fiber = pool.Acquire(func);
			{
				std::lock_guard<std::mutex> lock(readyMutex);
				ready.push_back(fiber);
			}
			cv.notify_one();
		}

		void Scheduler::WorkerLoop() {
			while (running) {
				Context* fiber = nullptr;
				{
					std::unique_lock<std::mutex> lock(readyMutex);
					cv.wait(lock, 
						[this]{ 
							return !ready.empty() || !running; 
						}
					);
					if (!running) {
						return;
					}
					fiber = ready.front();
					ready.pop_front();
				}

				current = fiber;
				if (!current->Finished()) {
					current->Resume();
				}

				if (!current->Finished()) {
					{
						std::lock_guard<std::mutex> lock(readyMutex);
						ready.push_back(fiber);
					}
					cv.notify_one();
				} else {
					pool.Release(fiber);
				}
			}
		}

		void Scheduler::Yield() {
	#if EWE_DEBUG
			assert(current);		
	#endif
			current->Yield();
		}

		void Scheduler::Run() {
			for (auto &w : workers) {
				w.join();
			}
		}

		void Scheduler::Stop() {
			running = false;
			cv.notify_all();
		}
	}
}
