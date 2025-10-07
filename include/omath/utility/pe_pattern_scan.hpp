//
// Created by Vlad on 10/7/2025.
//

#pragma once
#include <cstdint>
#include <optional>
#include <string_view>
#include <filesystem>
namespace omath
{
    class PePatternScanner final
    {
    public:
        [[nodiscard]]
        static std::optional<std::uintptr_t> scan_for_pattern_in_loaded_module(const std::string_view& module_name,
                                                                               const std::string_view& pattern);

        [[nodiscard]]
        static std::optional<std::uintptr_t> scan_for_pattern_in_file(const std::filesystem::path& path_to_file);
    };
} // namespace omath