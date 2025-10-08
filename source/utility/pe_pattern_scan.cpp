//
// Created by Vlad on 10/7/2025.
//
#include "omath/utility/pe_pattern_scan.hpp"
#include "omath/system/pe/section_header.hpp"
#include "omath/utility/pattern_scan.hpp"
#include <fstream>
#include <omath/system/pe/dos_header.hpp>
#include <span>
#include <stdexcept>
#ifdef _WIN32
#include <Windows.h>
#endif

namespace omath
{

    std::optional<std::uintptr_t>
    PePatternScanner::scan_for_pattern_in_loaded_module([[maybe_unused]] const std::string_view& module_name,
                                                        [[maybe_unused]] const std::string_view& pattern)
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
    std::optional<std::uintptr_t> PePatternScanner::scan_for_pattern_in_file(const std::filesystem::path& path_to_file,
                                                                             const std::string_view& pattern)
    {
        const auto pe_section = extract_section_from_pe_file(path_to_file, ".text");

        if (!pe_section.has_value())
            return std::nullopt;

        const auto scan_result = PatternScanner::scan_for_pattern(pe_section->cbegin(), pe_section->cend(), pattern);

        if (scan_result == pe_section->cend())
            return std::nullopt;

        return std::distance(pe_section->begin(), pe_section->end());
    }
    std::optional<std::vector<std::byte>>
    PePatternScanner::extract_section_from_pe_file([[maybe_unused]] const std::filesystem::path& path_to_file,
                                                   [[maybe_unused]] const std::string_view& section_name)
    {
#ifdef _WIN32
        std::fstream file(path_to_file, std::ios::binary | std::ios::in);

        if (!file.is_open()) [[unlikely]]
            return std::nullopt;

        system::pe::DosHeader dos_header{};
        file.read(reinterpret_cast<char*>(&dos_header), sizeof(dos_header));

        if (dos_header.e_magic != 0x5A4D) [[unlikely]]
            return std::nullopt;

        file.seekg(dos_header.e_lfanew, std::ios::beg);

        IMAGE_NT_HEADERS32 nt_headers{};
        file.read(reinterpret_cast<char*>(&nt_headers), sizeof(nt_headers));

        if (nt_headers.Signature != 0x00004550) [[unlikely]]
            return std::nullopt;

        constexpr size_t size_of_signature = 4;
        const auto offset_to_segment_table = dos_header.e_lfanew + nt_headers.FileHeader.SizeOfOptionalHeader
                                             + sizeof(IMAGE_FILE_HEADER) + size_of_signature;

        file.seekg(static_cast<std::fstream::off_type>(offset_to_segment_table), std::ios::beg);

        for (std::size_t i = 0; i < nt_headers.FileHeader.NumberOfSections; i++)
        {
            section_header_t current_section{};
            file.read(reinterpret_cast<char*>(&current_section), sizeof(IMAGE_SECTION_HEADER));

            if (std::string_view(current_section.name) != section_name)
                continue;

            std::vector<std::byte> section_data(current_section.size_raw_data);

            file.seekg(current_section.ptr_raw_data, std::ios::beg);
            file.read(reinterpret_cast<char*>(section_data.data()), static_cast<std::streamsize>(section_data.size()));

            return section_data;
        }
        return std::nullopt;
#else
        throw std::runtime_error("Pattern scan for loaded modules is only for windows platform");
#endif
    }
} // namespace omath