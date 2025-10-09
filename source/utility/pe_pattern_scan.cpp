//
// Created by Vlad on 10/7/2025.
//
#include "omath/utility/pe_pattern_scan.hpp"
#include "omath/system/pe/image_nt_headers.hpp"
#include "omath/system/pe/section_header.hpp"
#include "omath/utility/pattern_scan.hpp"
#include <fstream>
#include <omath/system/pe/dos_header.hpp>
#include <span>
#include <stdexcept>
#include <variant>
#ifdef _WIN32
#include <Windows.h>
#endif

using namespace omath::system::pe;
using NtHeaderVariant = std::variant<ImageNtHeaders<NtArchitecture::x64_bit>, ImageNtHeaders<NtArchitecture::x32_bit>>;

namespace
{
    [[nodiscard]]
    NtHeaderVariant get_nt_header_from_file(std::fstream& file, const DosHeader& dos_header)
    {
        ImageNtHeaders<NtArchitecture::x32_bit> x86_headers;
        file.seekg(dos_header.e_lfanew, std::ios::beg);
        file.read(reinterpret_cast<char*>(&x86_headers), sizeof(x86_headers));

        if (x86_headers.optional_header.magic == opt_hdr32_magic)
            return x86_headers;

        ImageNtHeaders<NtArchitecture::x64_bit> x64_headers;
        file.seekg(dos_header.e_lfanew, std::ios::beg);
        file.read(reinterpret_cast<char*>(&x64_headers), sizeof(x64_headers));

        return x64_headers;
    }

    [[nodiscard]]
    constexpr bool invalid_dos_header_file(const DosHeader& dos_header)
    {
        constexpr std::uint16_t dos_hdr_magic = 0x5A4D;
        return dos_header.e_magic != dos_hdr_magic;
    }
    [[nodiscard]]
    constexpr bool invalid_nt_header_file(const NtHeaderVariant& variant)
    {
        constexpr std::uint32_t nt_hdr_magic = 0x4550;
        return std::visit([](const auto& header) -> bool { return header.signature != nt_hdr_magic; }, variant);
    }
} // namespace

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
    std::optional<PeSectionScanResult>
    PePatternScanner::scan_for_pattern_in_file(const std::filesystem::path& path_to_file,
                                               const std::string_view& pattern)
    {
        const auto pe_section = extract_section_from_pe_file(path_to_file, ".text");

        if (!pe_section.has_value())
            return std::nullopt;

        const auto scan_result =
                PatternScanner::scan_for_pattern(pe_section->data.cbegin(), pe_section->data.cend(), pattern);

        if (scan_result == pe_section->data.cend())
            return std::nullopt;
        const auto offset = std::distance(pe_section->data.begin(), scan_result);

        return PeSectionScanResult{.virtual_base_addr = pe_section->virtual_base_addr,
                                   .raw_base_addr = pe_section->raw_base_addr,
                                   .target_offset = offset};
    }
    std::optional<PePatternScanner::Section>
    PePatternScanner::extract_section_from_pe_file([[maybe_unused]] const std::filesystem::path& path_to_file,
                                                   [[maybe_unused]] const std::string_view& section_name)
    {
        std::fstream file(path_to_file, std::ios::binary | std::ios::in);

        if (!file.is_open()) [[unlikely]]
            return std::nullopt;

        DosHeader dos_header{};
        file.read(reinterpret_cast<char*>(&dos_header), sizeof(dos_header));

        if (invalid_dos_header_file(dos_header)) [[unlikely]]
            return std::nullopt;

        const auto nt_headers = get_nt_header_from_file(file, dos_header);

        if (invalid_nt_header_file(nt_headers)) [[unlikely]]
            return std::nullopt;

        return std::visit(
                [&file, &dos_header, &section_name](auto& concrete_headers) -> std::optional<Section>
                {
                    constexpr std::size_t size_of_signature = sizeof(concrete_headers.signature);
                    const auto offset_to_segment_table = dos_header.e_lfanew
                                                         + concrete_headers.file_header.size_optional_header
                                                         + sizeof(FileHeader) + size_of_signature;

                    file.seekg(static_cast<std::fstream::off_type>(offset_to_segment_table), std::ios::beg);

                    for (std::size_t i = 0; i < concrete_headers.file_header.num_sections; i++)
                    {
                        SectionHeader current_section{};
                        file.read(reinterpret_cast<char*>(&current_section), sizeof(current_section));

                        if (std::string_view(current_section.name) != section_name)
                            continue;

                        std::vector<std::byte> section_data(current_section.size_raw_data);

                        file.seekg(current_section.ptr_raw_data, std::ios::beg);
                        file.read(reinterpret_cast<char*>(section_data.data()),
                                  static_cast<std::streamsize>(section_data.size()));
                        return Section{.virtual_base_addr = current_section.virtual_address
                                                            + concrete_headers.optional_header.image_base,
                                       .raw_base_addr = current_section.ptr_raw_data,
                                       .data = std::move(section_data)};
                    }
                    return std::nullopt;
                },
                nt_headers);
    }
} // namespace omath