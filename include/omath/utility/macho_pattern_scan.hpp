//
// Created by Copilot on 04.02.2026.
//
#pragma once
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string_view>
#include "section_scan_result.hpp"
namespace omath
{
    class MachOPatternScanner final
    {
    public:
        [[nodiscard]]
        static std::optional<std::uintptr_t>
        scan_for_pattern_in_loaded_module(const void* module_base_address, const std::string_view& pattern,
                                          const std::string_view& target_section_name = "__text");

        [[nodiscard]]
        static std::optional<SectionScanResult>
        scan_for_pattern_in_file(const std::filesystem::path& path_to_file, const std::string_view& pattern,
                                 const std::string_view& target_section_name = "__text");
    };
} // namespace omath
