//
// Created by Vlad on 10/7/2025.
//

#pragma once
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string_view>
namespace omath
{
    struct PeSectionScanResult
    {
        std::uint64_t virtual_base_addr;
        std::uint64_t raw_base_addr;
        std::ptrdiff_t target_offset;
    };
    class PePatternScanner final
    {
    public:
        [[nodiscard]]
        static std::optional<std::uintptr_t> scan_for_pattern_in_loaded_module(const std::string_view& module_name,
                                                                               const std::string_view& pattern);

        [[nodiscard]]
        static std::optional<PeSectionScanResult>
        scan_for_pattern_in_file(const std::filesystem::path& path_to_file, const std::string_view& pattern,
                                 const std::string_view& target_section_name = ".text");
    };
} // namespace omath