//
// Created by orange on 10.03.2026.
//
#ifdef OMATH_ENABLE_LUA
#include "omath/lua/lua.hpp"
#include <format>
#include <omath/utility/elf_pattern_scan.hpp>
#include <omath/utility/macho_pattern_scan.hpp>
#include <omath/utility/pattern_scan.hpp>
#include <omath/utility/pe_pattern_scan.hpp>
#include <omath/utility/section_scan_result.hpp>
#include <sol/sol.hpp>
#endif

namespace omath::lua
{
    void LuaInterpreter::register_pattern_scan(sol::table& omath_table)
    {
        omath_table.new_usertype<SectionScanResult>(
                "SectionScanResult", sol::no_constructor, "virtual_base_addr",
                sol::readonly(&SectionScanResult::virtual_base_addr), "raw_base_addr",
                sol::readonly(&SectionScanResult::raw_base_addr), "target_offset",
                sol::readonly(&SectionScanResult::target_offset), sol::meta_function::to_string,
                [](const SectionScanResult& r)
                {
                    return std::format("SectionScanResult(vbase=0x{:X}, raw_base=0x{:X}, offset={})",
                                       r.virtual_base_addr, r.raw_base_addr, r.target_offset);
                });

        // Generic scanner: accepts a Lua string as a byte buffer
        auto ps_table = omath_table["PatternScanner"].get_or_create<sol::table>();
        ps_table["scan"] = [](const std::string& data, const std::string& pattern) -> sol::optional<std::ptrdiff_t>
        {
            const auto* begin = reinterpret_cast<const std::byte*>(data.data());
            const auto* end = begin + data.size();
            const auto* result = PatternScanner::scan_for_pattern(begin, end, pattern);
            if (result == end)
                return sol::nullopt;
            return std::distance(begin, result);
        };

        auto pe_table = omath_table["PePatternScanner"].get_or_create<sol::table>();
        pe_table["scan_in_module"] = [](std::uintptr_t base_addr, const std::string& pattern,
                                        sol::optional<std::string> section) -> sol::optional<std::uintptr_t>
        {
            auto result = PePatternScanner::scan_for_pattern_in_loaded_module(reinterpret_cast<const void*>(base_addr),
                                                                              pattern, section.value_or(".text"));
            if (!result)
                return sol::nullopt;
            return *result;
        };
        pe_table["scan_in_file"] = [](const std::string& path, const std::string& pattern,
                                      sol::optional<std::string> section) -> sol::optional<SectionScanResult>
        {
            auto result = PePatternScanner::scan_for_pattern_in_file(std::filesystem::path(path), pattern,
                                                                     section.value_or(".text"));
            if (!result)
                return sol::nullopt;
            return *result;
        };
        auto elf_table = omath_table["ElfPatternScanner"].get_or_create<sol::table>();
        elf_table["scan_in_module"] = [](std::uintptr_t base_addr, const std::string& pattern,
                                         sol::optional<std::string> section) -> sol::optional<std::uintptr_t>
        {
            auto result = ElfPatternScanner::scan_for_pattern_in_loaded_module(reinterpret_cast<const void*>(base_addr),
                                                                               pattern, section.value_or(".text"));
            if (!result)
                return sol::nullopt;
            return *result;
        };
        elf_table["scan_in_file"] = [](const std::string& path, const std::string& pattern,
                                       sol::optional<std::string> section) -> sol::optional<SectionScanResult>
        {
            auto result = ElfPatternScanner::scan_for_pattern_in_file(std::filesystem::path(path), pattern,
                                                                      section.value_or(".text"));
            if (!result)
                return sol::nullopt;
            return *result;
        };
        auto macho_table = omath_table["MachOPatternScanner"].get_or_create<sol::table>();
        macho_table["scan_in_module"] = [](std::uintptr_t base_addr, const std::string& pattern,
                                           sol::optional<std::string> section) -> sol::optional<std::uintptr_t>
        {
            auto result = MachOPatternScanner::scan_for_pattern_in_loaded_module(
                    reinterpret_cast<const void*>(base_addr), pattern, section.value_or("__text"));
            if (!result)
                return sol::nullopt;
            return *result;
        };
        macho_table["scan_in_file"] = [](const std::string& path, const std::string& pattern,
                                         sol::optional<std::string> section) -> sol::optional<SectionScanResult>
        {
            auto result = MachOPatternScanner::scan_for_pattern_in_file(std::filesystem::path(path), pattern,
                                                                        section.value_or("__text"));
            if (!result)
                return sol::nullopt;
            return *result;
        };
    }
} // namespace omath::lua