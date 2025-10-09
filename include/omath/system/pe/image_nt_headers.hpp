//
// Created by Vlad on 10/9/2025.
//

#pragma once
#include "file_header.hpp"
#include "optional_header.hpp"

namespace omath::system::pe
{
    enum class NtArchitecture
    {
        x32_bit,
        x64_bit,
    };
    template<NtArchitecture architecture>
    struct ImageNtHeaders
    {
        std::uint32_t signature;
        FileHeader file_header;
        OptionalHeader<architecture == NtArchitecture::x64_bit> optional_header;
    };
} // namespace omath::system::pe