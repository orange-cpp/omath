#pragma once

#include <cstdint>

#if defined _M_IX86 || defined __i386__
#   define SUBHOOK_X86
#   define SUBHOOK_BITS 32
#elif defined _M_AMD64 || defined __amd64__
#   define SUBHOOK_X86_64
#   define SUBHOOK_BITS 64
#else
#   error Unsupported architecture
#endif

#if defined _WIN32 || defined __CYGWIN__
#   define SUBHOOK_WINDOWS
#elif defined __linux__ || defined __FreeBSD__ || defined __OpenBSD__ || defined __NetBSD__
#   define SUBHOOK_UNIX
#elif defined __APPLE__
#   define SUBHOOK_APPLE
#   define SUBHOOK_UNIX
#else
#   error Unsupported operating system
#endif

namespace omath::hooking
{
    typedef enum subhook_flags
    {
        /*
         * Use the 64-bit jump method on x86-64. Unlike the classical 32-bit JMP,
         * this approach ensures that the destination code can be reached from any
         * point in the 64-bit address space, even if the source and destination are
         * more than 4GB away from each other (meaning we are not limited to using
         * JMP 32-bit offsets).
         *
         * Keep in mind that it requires overwriting a few more leading instructions
         * inside the target code, thus it may not work with extremely short
         * functions (14 bytes vs 5 bytes).
         *
         * Credits to @Ozymandias117 and @RomanHargrave on GitHub for implementing
         * this in subhook.
         */
        SUBHOOK_64BIT_OFFSET = 0x01,
        /*
         * Generate a trampoline for jumping back to the original code faster (without
         * removing the hook each time).
         *
         * In some scenarios, trampolines cannot be created. See "Known limitations"
         * in the README file.
         */
        SUBHOOK_TRAMPOLINE = 0x02,
        /*
         * Windows x64 only: Try to allocate a trampoline buffer within +/- 2GB range
         * of the original function to overcome a possible issue with relocating memory
         * referencing instructions, particularly those which use RIP-relative
         * addresses (i.e. with 32-bit offsets).
         *
         * Caution: this feature may slow down your code.
         */
        SUBHOOK_TRAMPOLINE_ALLOC_NEARBY = 0x04
    } subhook_flags_t;

    struct subhook_struct;
    typedef struct subhook_struct* subhook_t;

    typedef int ( * subhook_disasm_handler_t)(void* src, int* reloc_op_offset);

    subhook_t subhook_new(void* src, void* dst, subhook_flags_t flags);

    void subhook_free(subhook_t hook);

    void* subhook_get_src(subhook_t hook);

    void* subhook_get_dst(subhook_t hook);

    void* subhook_get_trampoline(subhook_t hook);

    int subhook_install(subhook_t hook);

    [[nodiscard]]
    bool subhook_is_installed(subhook_t hook);

    int subhook_remove(subhook_t hook);
    void* ReadDestination(void* src);
    int Disassemble(void* src, int* reloc_op_offset);
    void subhook_set_disasm_handler(subhook_disasm_handler_t handler);


    enum HookFlags
    {
        HookNoFlags = 0,
        HookFlag64BitOffset = SUBHOOK_64BIT_OFFSET,
        HookFlagTrampoline = SUBHOOK_TRAMPOLINE,
        HookFlagTrampolineAllocNearby = SUBHOOK_TRAMPOLINE_ALLOC_NEARBY
    };

    inline HookFlags operator|(HookFlags o1, HookFlags o2)
    {
        return static_cast<HookFlags>(
                static_cast<unsigned int>(o1) | static_cast<unsigned int>(o2));
    }

    inline HookFlags operator&(HookFlags o1, HookFlags o2)
    {
        return static_cast<HookFlags>(
                static_cast<unsigned int>(o1) & static_cast<unsigned int>(o2));
    }

    inline void* ReadHookDst(void* src)
    {
        return ReadDestination(src);
    }

    inline void SetDisasmHandler(subhook_disasm_handler_t handler)
    {
        subhook_set_disasm_handler(handler);
    }

    class Hook
    {
    public:
        Hook() : hook_(nullptr)
        {}

        Hook(void* src, void* dst, HookFlags flags = HookNoFlags)
        : hook_(subhook_new(src, dst, (subhook_flags_t) flags))
        {
        }

        ~Hook()
        {
            subhook_remove(hook_);
            subhook_free(hook_);
        }

        [[nodiscard]] void* GetSrc() const
        {
            return subhook_get_src(hook_);
        }

        [[nodiscard]] void* GetDst() const
        {
            return subhook_get_dst(hook_);
        }

        [[nodiscard]] void* GetTrampoline() const
        {
            return subhook_get_trampoline(hook_);
        }

        bool Install()
        {
            return subhook_install(hook_) == 0;
        }

        bool Install(void* src, void* dst, HookFlags flags = HookNoFlags)
        {
            if (hook_ != nullptr)
            {
                subhook_remove(hook_);
                subhook_free(hook_);
            }

            hook_ = subhook_new(src, dst, (subhook_flags_t) flags);

            if (hook_ == nullptr)
                return false;

            return Install();
        }

        bool Remove()
        {
            return subhook_remove(hook_) == 0;
        }

        [[nodiscard]] bool IsInstalled() const
        {
            return subhook_is_installed(hook_);
        }

    private:
        //Hook(const Hook &);
        //void operator=(const Hook &);

    private:
        subhook_t hook_;
    };

    class ScopedHookRemove
    {
    public:
        explicit ScopedHookRemove(Hook* hook) : hook_(hook), removed_(hook_->Remove())
        {

        }

        ~ScopedHookRemove()
        {
            if (removed_)
                hook_->Install();
        }

        ScopedHookRemove(const ScopedHookRemove &) = delete;
        void operator=(const ScopedHookRemove &) = delete;

    private:
        Hook* hook_;
        bool removed_;
    };

    class ScopedHookInstall
    {
    public:
        explicit ScopedHookInstall(Hook* hook) : hook_(hook), installed_(hook_->Install())
        {

        }

        ScopedHookInstall(Hook* hook, void* src, void* dst, HookFlags flags = HookNoFlags)
        : hook_(hook), installed_(hook_->Install(src, dst, flags))
        {
        }

        ~ScopedHookInstall()
        {
            if (installed_)
                hook_->Remove();
        }

    private:
        ScopedHookInstall(const ScopedHookInstall &);

        void operator=(const ScopedHookInstall &);

    private:
        Hook* hook_;
        bool installed_;
    };

} // namespace subhook