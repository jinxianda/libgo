#pragma once

#include "../common/config.h"
#include "fcontext.h"

#if defined(LIBGO_SYS_Windows)
# include "fiber/context.h"
#else
namespace co {

class Context {
public:
    Context(fn_t fn, intptr_t vp, std::size_t stackSize)
            : fn_(fn), vp_(vp), stackSize_(stackSize) {
        stack_ = (char *) StackTraits::MallocFunc()(stackSize_);
        DebugPrint(dbg_task, "valloc stack. size=%u ptr=%p",
                   stackSize_, stack_);

        ctx_ = libgo_make_fcontext(stack_ + stackSize_, stackSize_, &Context::StaticRun);

        int protectPage = StackTraits::GetProtectStackPageSize();
        if (protectPage && StackTraits::ProtectStack(stack_, stackSize_, protectPage))
            protectPage_ = protectPage;
    }

    ~Context() {
        if (stack_) {
            DebugPrint(dbg_task, "free stack. ptr=%p", stack_);
            if (protectPage_)
                StackTraits::UnprotectStack(stack_, protectPage_);
            StackTraits::FreeFunc()(stack_);
            stack_ = NULL;
        }
    }

    ALWAYS_INLINE void SwapIn() {
        ctx_ = libgo_jump_fcontext(ctx_, this).fctx;
    }

    ALWAYS_INLINE void SwapTo(Context &other) {
        libgo_jump_fcontext(other.ctx_, &other);
    }

    ALWAYS_INLINE void SwapOut() {
        GetTlsContext() = libgo_jump_fcontext(GetTlsContext(), 0).fctx;
    }

    static fcontext_t &GetTlsContext() {
        static thread_local fcontext_t tls_context;
        return tls_context;
    }

    ALWAYS_INLINE static void FCONTEXT_CALL StaticRun(transfer_t transfer) {
        GetTlsContext() = transfer.fctx;
        Context *ctx = (Context *) transfer.data;
        ctx->fn_(ctx->vp_);
    }

private:
    fcontext_t ctx_;
    fn_t fn_;
    intptr_t vp_;
    char *stack_ = nullptr;
    uint32_t stackSize_ = 0;
    int protectPage_ = 0;
};
} // namespace co

#endif
