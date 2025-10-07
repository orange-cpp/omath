//
// Created by Vlad on 10/7/2025.
//
#include "omath/utility/pe_pattern_scan.hpp"
#include "omath/utility/pattern_scan.hpp"
#include <span>
#include <stdexcept>
#ifdef _WIN32
#include <Windows.h>
#endif

namespace omath
{

    std::optional<std::uintptr_t>
    PePatternScanner::scan_for_pattern_in_loaded_module(const std::string_view& module_name,
                                                        const std::string_view& pattern)
    {
#ifdef _WIN32
        const auto base_address = reinterpret_cast<std::uintptr_t>(GetModuleHandleA(module_name.data()));

        if (!base_address)
            return std::nullopt;

        const auto dos_headers = reinterpret_cast<PIMAGE_DOS_HEADER>(base_address);
        const auto image_nt_headers = reinterpret_cast<PIMAGE_NT_HEADERS>(base_address + dos_headers->e_lfanew);

        // Define .code segment as scan area
        const auto start = image_nt_headers->OptionalHeader.BaseOfCode;
        const auto scan_size = image_nt_headers->OptionalHeader.SizeOfCode;

        const auto scan_range = std::span{reinterpret_cast<std::byte*>(base_address) + start, scan_size};

        const auto result = PatternScanner::scan_for_pattern(scan_range, pattern);

        if (result != scan_range.cend())
            return reinterpret_cast<std::uintptr_t>(&*result);

        return std::nullopt;
#else
        throw std::runtime_error("Pattern scan for loaded modules is only for windows platform");
#endif
    }
    std::optional<std::uintptr_t> PePatternScanner::scan_for_pattern_in_file(const std::filesystem::path& path_to_file)
    {
    }
} // namespace omath