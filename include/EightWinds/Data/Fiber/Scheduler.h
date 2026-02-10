#pragma once

#include <deque>
#include <thread>
#include <array>
#include <condition_variable>
#include <mutex>

namespace EWE{
	namespace Fiber{
		
		enum class Priority { 
			High = 0, 
			Medium = 1, 
			Low = 2,
			
			COUNT
		};
		
		class Scheduler {
		public:
			[[nodiscard]] explicit Scheduler(std::size_t threads = std::thread::hardware_concurrency(), uint8_t fibersPerThread = 4);

			void Spawn(Priority priority, std::function<void()> func);
			void Run();
			void Stop();

			void Yield();

		private:
			void WorkerLoop();

			std::array<std::deque<Context*>, Priority::COUNT> ready;
			std::mutex readyMutex;
			std::condition_variable cv;
			std::vector<std::thread> workers;
			Pool pool;
			bool running = true;
			thread_local static Context* current;
		};
	} //namespace Fiber
} //namespace EWE