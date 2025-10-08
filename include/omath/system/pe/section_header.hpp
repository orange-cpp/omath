//
// Created by Vlad on 10/8/2025.
//

#pragma once
#include <cstdint>
union section_characteristics_t
{
    std::uint32_t flags;
    struct
    {
        std::uint32_t _pad0 : 5;
        std::uint32_t cnt_code : 1; // Section contains code.
        std::uint32_t cnt_init_data : 1; // Section contains initialized data.
        std::uint32_t cnt_uninit_data : 1; // Section contains uninitialized data.
        std::uint32_t _pad1 : 1;
        std::uint32_t lnk_info : 1; // Section contains comments or some other type of information.
        std::uint32_t _pad2 : 1;
        std::uint32_t lnk_remove : 1; // Section contents will not become part of image.
        std::uint32_t lnk_comdat : 1; // Section contents comdat.
        std::uint32_t _pad3 : 1;
        std::uint32_t no_defer_spec_exc : 1; // Reset speculative exceptions handling bits in the TLB entries for this
                                             // section.
        std::uint32_t mem_far : 1;
        std::uint32_t _pad4 : 1;
        std::uint32_t mem_purgeable : 1;
        std::uint32_t mem_locked : 1;
        std::uint32_t mem_preload : 1;
        std::uint32_t alignment : 4; // Alignment calculated as: n ? 1 << ( n - 1 ) : 16
        std::uint32_t lnk_nreloc_ovfl : 1; // Section contains extended relocations.
        std::uint32_t mem_discardable : 1; // Section can be discarded.
        std::uint32_t mem_not_cached : 1; // Section is not cachable.
        std::uint32_t mem_not_paged : 1; // Section is not pageable.
        std::uint32_t mem_shared : 1; // Section is shareable.
        std::uint32_t mem_execute : 1; // Section is executable.
        std::uint32_t mem_read : 1; // Section is readable.
        std::uint32_t mem_write : 1; // Section is writeable.
    };
};

// Section header
//
struct section_header_t
{
    char name[8];
    union
    {
        std::uint32_t physical_address;
        std::uint32_t virtual_size;
    };
    std::uint32_t virtual_address;

    std::uint32_t size_raw_data;
    std::uint32_t ptr_raw_data;

    std::uint32_t ptr_relocs;
    std::uint32_t ptr_line_numbers;
    uint16_t num_relocs;
    uint16_t num_line_numbers;

    section_characteristics_t characteristics;
};