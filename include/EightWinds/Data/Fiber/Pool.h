#pragma once

#include "EightWinds/Data/Fiber/Context.h"

#include <vector>

namespace EWE{
	namespace Fiber{
		class Pool { 
		public:
			[[nodiscard]] explicit Pool(std::size_t poolSize, std::size_t stackSize = 64 * 1024);

			Context* Acquire(std::function<void()> f);
			void Release(Context* f);

		private:
			std::vector<Context*> pool;
			std::size_t stackSize;
		};
	}//namespace Fiber
}//namespace EWE