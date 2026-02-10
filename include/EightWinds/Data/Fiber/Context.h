#pragma once

#include "EightWinds/Data/HeapBlock.h"
#include "EightWinds/Preprocessor.h"

#include <boost/context/detail/fcontext.hpp>

#include <functional>

namespace EWE{
	namespace Fiber{
		class Context {
		public:

			[[nodiscard]] Context(Func f, std::size_t stackSize = 64 * 1024);
			~Context();

			void Resume();
			void Yield();

			bool Finished() const { return done; }

		private:
			static void Trampoline(boost::context::detail::transfer_t t);

			boost::context::detail::fcontext_t ctx;
			boost::context::detail::fcontext_t* callerCtx = nullptr;
			HeapBlock<uint8_t> stack = nullptr;
			std::function<void()> func;
			bool done = false;
		};
	} //namespace Fiber
}