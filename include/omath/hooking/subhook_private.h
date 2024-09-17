#pragma once
#include <cstddef>


namespace omath::hooking
{
    struct subhook_struct
    {
        int installed;
        void* src;
        void* dst;
        subhook_flags_t flags;
        void* code;
        void* trampoline;
        size_t jmp_size;
        size_t trampoline_size;
        size_t trampoline_len;
    };


    [[nodiscard]] int subhook_unprotect(void* address, size_t size);

    void* subhook_alloc_code(void* target_address, size_t size);

    int subhook_free_code(void* address, size_t size);
}