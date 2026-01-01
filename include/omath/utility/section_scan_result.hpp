//
// Created by Vladislav on 01.01.2026.
//

#pragma once
#include <cstddef>
#include <cstdint>
namespace omath
{
    struct SectionScanResult final
    {
        std::uintptr_t virtual_base_addr;
        std::uintptr_t raw_base_addr;
        std::ptrdiff_t target_offset;
    };
}