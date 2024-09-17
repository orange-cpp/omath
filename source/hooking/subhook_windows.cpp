#include <cstdint>
#include <windows.h>


#define SUBHOOK_CODE_PROTECT_FLAGS PAGE_EXECUTE_READWRITE

namespace omath::hooking
{
    int subhook_unprotect(void* address, size_t size)
    {
        DWORD old_flags;
        BOOL result = VirtualProtect(address, size, SUBHOOK_CODE_PROTECT_FLAGS, &old_flags);
        return !result;
    }

    void* subhook_alloc_code([[maybe_unused]]void* target_address, size_t size)
    {
#if defined _M_AMD64 || defined __amd64__
        if (target_address != nullptr)
        {
            SYSTEM_INFO sys_info;
            DWORD page_size;
            int64_t offset;

            GetSystemInfo(&sys_info);
            page_size = sys_info.dwPageSize;

            int64_t pivot = (int64_t) target_address & ~((int64_t) page_size - 1);
            void* result;

            for (offset = page_size; offset <= ((int64_t) 1 << 31); offset += page_size)
            {
                result = VirtualAlloc((void*) (pivot - offset), size, MEM_COMMIT | MEM_RESERVE,
                                      SUBHOOK_CODE_PROTECT_FLAGS);

                if (result != nullptr)
                    return result;

                result = VirtualAlloc((void*) (pivot + offset), size, MEM_COMMIT | MEM_RESERVE,
                                      SUBHOOK_CODE_PROTECT_FLAGS);
                if (result != nullptr)
                    return result;
            }
        }
#endif
        return VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, SUBHOOK_CODE_PROTECT_FLAGS);
    }

    int subhook_free_code(void* address, [[maybe_unused]] size_t size)
    {
        if (address == nullptr)
            return 0;

        return !VirtualFree(address, 0, MEM_RELEASE);
    }
}