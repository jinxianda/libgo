#pragma once

#include "../common/config.h"

namespace co {

struct StackTraits {
    static stack_malloc_fn_t &MallocFunc();

    static stack_free_fn_t &FreeFunc();

    static int &GetProtectStackPageSize();

    static bool ProtectStack(void *stack, std::size_t size, int pageSize);

    static void UnprotectStack(void *stack, int pageSize);
};

} // namespace co

//#include "../../third_party/boost.context/boost/context/fcontext.hpp"
//using boost::context::fcontext_t;
//using boost::context::jump_fcontext;
//using boost::context::make_fcontext;

extern "C"
{

typedef void *fcontext_t;

struct transfer_t {
    fcontext_t fctx;
    void *data;
};

typedef void (*fn_t)(intptr_t);

typedef void (FCONTEXT_CALL *fctx_t)(transfer_t);

transfer_t libgo_jump_fcontext(fcontext_t const to, void *vp);

fcontext_t libgo_make_fcontext(void *sp, std::size_t size, fctx_t fn);

} // extern "C"
