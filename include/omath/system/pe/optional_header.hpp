//
// Created by Vlad on 10/8/2025.
//
#pragma once
#include <cstdint>

namespace omath::system::pe
{
    static constexpr std::uint16_t opt_hdr32_magic = 0x010B;
    static constexpr std::uint16_t opt_hdr64_magic = 0x020B;
    enum class SubsystemId : uint16_t
    {
        unknown = 0x0000, // Unknown subsystem.
        native = 0x0001, // Image doesn't require a subsystem.
        windows_gui = 0x0002, // Image runs in the Windows GUI subsystem.
        windows_cui = 0x0003, // Image runs in the Windows character subsystem
        os2_cui = 0x0005, // image runs in the OS/2 character subsystem.
        posix_cui = 0x0007, // image runs in the Posix character subsystem.
        native_windows = 0x0008, // image is a native Win9x driver.
        windows_ce_gui = 0x0009, // Image runs in the Windows CE subsystem.
        efi_application = 0x000A, //
        efi_boot_service_driver = 0x000B, //
        efi_runtime_driver = 0x000C, //
        efi_rom = 0x000D,
        xbox = 0x000E,
        windows_boot_application = 0x0010,
        xbox_code_catalog = 0x0011,
    };

    struct DataDirectory
    {
        uint32_t rva;
        uint32_t size;
    };
    struct OptionalHeaderX64
    {
        // Standard fields.
        uint16_t magic;
        std::uint16_t linker_version;

        uint32_t size_code;
        uint32_t size_init_data;
        uint32_t size_uninit_data;

        uint32_t entry_point;
        uint32_t base_of_code;

        // NT additional fields.
        uint64_t image_base;
        uint32_t section_alignment;
        uint32_t file_alignment;

        std::uint32_t os_version;
        std::uint32_t img_version;
        std::uint32_t subsystem_version;
        uint32_t win32_version_value;

        uint32_t size_image;
        uint32_t size_headers;

        uint32_t checksum;
        SubsystemId subsystem;
        SubsystemId characteristics;

        uint64_t size_stack_reserve;
        uint64_t size_stack_commit;
        uint64_t size_heap_reserve;
        uint64_t size_heap_commit;

        uint32_t ldr_flags;

        uint32_t num_data_directories;
        DataDirectory data_directories[16];
    };
    struct OptionalHeaderX86
    {
        // Standard fields.
        uint16_t magic;
        uint16_t linker_version;

        uint32_t size_code;
        uint32_t size_init_data;
        uint32_t size_uninit_data;

        uint32_t entry_point;
        uint32_t base_of_code;
        uint32_t base_of_data;

        // NT additional fields.
        uint32_t image_base;
        uint32_t section_alignment;
        uint32_t file_alignment;

        std::uint32_t os_version;
        std::uint32_t img_version;
        std::uint32_t subsystem_version;
        uint32_t win32_version_value;

        uint32_t size_image;
        uint32_t size_headers;

        uint32_t checksum;
        SubsystemId subsystem;
        std::uint16_t characteristics;

        uint32_t size_stack_reserve;
        uint32_t size_stack_commit;
        uint32_t size_heap_reserve;
        uint32_t size_heap_commit;

        uint32_t ldr_flags;

        uint32_t num_data_directories;
        DataDirectory data_directories[16];
    };
    template<bool x64 = true>
    using OptionalHeader = std::conditional_t<x64, OptionalHeaderX64, OptionalHeaderX86>;
} // namespace omath::system::pe