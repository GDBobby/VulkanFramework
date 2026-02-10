#include "EightWinds/Data/Fiber/Context.h"

namespace EWE{
	namespace Fiber{
		Context::Context(std::function<void()> f, std::size_t stackSize) 
		: func(std::move(f)) 
			stack{stackSize}
		{
			ctx = boost::context::detail::make_fcontext(
				static_cast<char*>(stack) + stackSize, stackSize, trampoline
			);
		}

		Context::~Context() {
			
		}

		void Context::Resume() {
			auto t = boost::context::detail::jump_fcontext(&ctx, callerCtx);
			callerCtx = t.fctx;
		}

		void Context::Yield() {
			auto t = boost::context::detail::jump_fcontext(callerCtx, ctx);
			ctx = t.fctx;
		}

		void Context::Trampoline(boost::context::detail::transfer_t t) {
			Context* self = static_cast<Context*>(t.data);
			self->callerCtx = t.fctx;
			self->func();
			self->done = true;
			boost::context::detail::jump_fcontext(self->callerCtx, self->ctx);
		}
		
	} //namespace Fiber
} //namespace EWE