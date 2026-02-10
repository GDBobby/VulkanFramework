#include "EightWinds/Data/Fiber/Pool.h"

namespace EWE{
	namespace Fiber{
		
		Pool::Pool(std::size_t poolSize, std::size_t stackSize)
		: stackSize(stackSize)
		{
			pool.reserve(poolSize);
			for (std::size_t i = 0; i < poolSize; i++) {
				pool.push_back(std::make_unique<Context>(nullptr, stackSize));
			}
		}

		Context* Pool::Acquire(std::function<void()> f) {
			if (pool.empty()) {
				return new Context(f, stackSize);
			}
			auto ctx = pool.back();
			pool.pop_back();
			*ctx = Context(f, stackSize);
			return ctx;
		}

		void Pool::Release(Context* f) {
			pool.push_back(f);
		}
		
	} //namespace Fiber
} //namespace EWE