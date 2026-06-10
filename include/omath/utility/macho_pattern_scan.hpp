//
// Created by Copilot on 04.02.2026.
//
#pragma once
#include "pattern_scan.hpp"
#include "section_scan_result.hpp"
#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <string_view>
namespace omath
{
    class MachOPatternScanner final
    {
    public:
        [[nodiscard]]
        static std::optional<std::uintptr_t>
        scan_for_pattern_in_loaded_module(const void* module_base_address, const std::string_view& pattern,
                                          const std::string_view& target_section_name = "__text");

        template<PatternScanner::fixed_string Pattern>
        [[nodiscard]]
        static std::optional<std::uintptr_t>
        scan_for_pattern_in_loaded_module(const void* module_base_address,
                                          const std::string_view& target_section_name = "__text")
        {
            return scan_for_pattern_in_loaded_module(module_base_address, target_section_name,
                                                     &MachOPatternScanner::scan_section_for_pattern<Pattern>);
        }

        [[nodiscard]]
        static std::optional<SectionScanResult>
        scan_for_pattern_in_file(const std::filesystem::path& path_to_file, const std::string_view& pattern,
                                 const std::string_view& target_section_name = "__text");

        template<PatternScanner::fixed_string Pattern>
        [[nodiscard]]
        static std::optional<SectionScanResult>
        scan_for_pattern_in_file(const std::filesystem::path& path_to_file,
                                 const std::string_view& target_section_name = "__text")
        {
            return scan_for_pattern_in_file(path_to_file, target_section_name,
                                            &MachOPatternScanner::scan_section_for_pattern<Pattern>);
        }

        [[nodiscard]]
        static std::optional<SectionScanResult>
        scan_for_pattern_in_memory_file(std::span<const std::byte> file_data, const std::string_view& pattern,
                                        const std::string_view& target_section_name = "__text");

        template<PatternScanner::fixed_string Pattern>
        [[nodiscard]]
        static std::optional<SectionScanResult>
        scan_for_pattern_in_memory_file(std::span<const std::byte> file_data,
                                        const std::string_view& target_section_name = "__text")
        {
            return scan_for_pattern_in_memory_file(file_data, target_section_name,
                                                   &MachOPatternScanner::scan_section_for_pattern<Pattern>);
        }

    private:
        using SectionScanFunction = std::optional<std::ptrdiff_t> (*)(std::span<const std::byte>);

        template<PatternScanner::fixed_string Pattern>
        [[nodiscard]]
        static std::optional<std::ptrdiff_t> scan_section_for_pattern(const std::span<const std::byte> section_data)
        {
            const auto result = PatternScanner::scan_for_pattern<Pattern>(section_data.begin(), section_data.end());

            if (result == section_data.end())
                return std::nullopt;

            return result - section_data.begin();
        }

        [[nodiscard]]
        static std::optional<std::uintptr_t>
        scan_for_pattern_in_loaded_module(const void* module_base_address, const std::string_view& target_section_name,
                                          SectionScanFunction scan_pattern);

        [[nodiscard]]
        static std::optional<SectionScanResult> scan_for_pattern_in_file(const std::filesystem::path& path_to_file,
                                                                         const std::string_view& target_section_name,
                                                                         SectionScanFunction scan_pattern);

        [[nodiscard]]
        static std::optional<SectionScanResult>
        scan_for_pattern_in_memory_file(std::span<const std::byte> file_data,
                                        const std::string_view& target_section_name, SectionScanFunction scan_pattern);
    };
} // namespace omath
