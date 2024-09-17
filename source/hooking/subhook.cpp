#include "omath/hooking/subhook.h"
#include "omath/hooking/subhook_private.h"

namespace omath::hooking
{
    subhook_disasm_handler_t subhook_disasm_handler = NULL;

    void* subhook_get_src(subhook_t hook)
    {
        return hook ? hook->src : nullptr;
    }

    void* subhook_get_dst(subhook_t hook)
    {
        return hook ? hook->dst : nullptr;
    }

    void* subhook_get_trampoline(subhook_t hook)
    {
        return hook ? hook->trampoline : nullptr;
    }

    bool subhook_is_installed(subhook_t hook)
    {
        return hook && hook->installed;
    }

    void subhook_set_disasm_handler(subhook_disasm_handler_t handler)
    {
        subhook_disasm_handler = handler;
    }
}
