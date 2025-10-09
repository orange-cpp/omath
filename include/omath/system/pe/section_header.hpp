//
// Created by Vlad on 10/8/2025.
//

#pragma once
#include <cstdint>

// Section header
//
namespace omath::system::pe
{
    struct SectionHeader final
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
        std::uint32_t  num_relocs;
        std::uint32_t  num_line_numbers;

        std::uint32_t characteristics;
    };
}