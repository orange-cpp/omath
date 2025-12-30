//
// Created by Vlad on 10/7/2025.
//
#include "omath/utility/pe_pattern_scan.hpp"
#include "omath/utility/pattern_scan.hpp"
#include <fstream>
#include <span>
#include <stdexcept>
#include <variant>

// Internal PE shit defines
// Big thx for linuxpe sources as ref
// Link: https://github.com/can1357/linux-pe
namespace
{
    constexpr std::uint16_t opt_hdr32_magic = 0x010B;
    constexpr std::uint16_t opt_hdr64_magic = 0x020B;

    // Standard fields.
    // ReSharper disable CppDeclaratorNeverUsed
    struct DataDirectory final
    {
        std::uint32_t rva;
        std::uint32_t size;
    };
    struct OptionalHeaderX64 final
    {
        std::uint16_t magic;

        std::uint16_t linker_version;

        std::uint32_t size_code;
        std::uint32_t size_init_data;
        std::uint32_t size_uninit_data;

        std::uint32_t entry_point;
        std::uint32_t base_of_code;

        // NT additional fields.
        std::uint64_t image_base;
        std::uint32_t section_alignment;
        std::uint32_t file_alignment;

        std::uint32_t os_version;
        std::uint32_t img_version;
        std::uint32_t subsystem_version;
        std::uint32_t win32_version_value;

        std::uint32_t size_image;
        std::uint32_t size_headers;

        std::uint32_t checksum;
        std::uint16_t subsystem;
        std::uint16_t characteristics;

        std::uint64_t size_stack_reserve;
        std::uint64_t size_stack_commit;
        std::uint64_t size_heap_reserve;
        std::uint64_t size_heap_commit;

        std::uint32_t ldr_flags;

        std::uint32_t num_data_directories;
        DataDirectory data_directories[16];
    };
    struct OptionalHeaderX86 final
    {
        // Standard fields.
        std::uint16_t magic{};
        std::uint16_t linker_version{};

        std::uint32_t size_code{};
        std::uint32_t size_init_data{};
        std::uint32_t size_uninit_data{};

        std::uint32_t entry_point{};
        std::uint32_t base_of_code{};
        std::uint32_t base_of_data{};

        // NT additional fields.
        std::uint32_t image_base{};
        std::uint32_t section_alignment{};
        std::uint32_t file_alignment{};

        std::uint32_t os_version{};
        std::uint32_t img_version{};
        std::uint32_t subsystem_version{};
        std::uint32_t win32_version_value{};

        std::uint32_t size_image{};
        std::uint32_t size_headers{};

        std::uint32_t checksum{};
        std::uint16_t subsystem{};
        std::uint16_t characteristics{};

        std::uint32_t size_stack_reserve{};
        std::uint32_t size_stack_commit{};
        std::uint32_t size_heap_reserve{};
        std::uint32_t size_heap_commit{};

        std::uint32_t ldr_flags{};

        std::uint32_t num_data_directories{};
        DataDirectory data_directories[16]{};
    };
    template<bool x64 = true>
    using OptionalHeader = std::conditional_t<x64, OptionalHeaderX64, OptionalHeaderX86>;

    struct FileHeader final
    {
        std::uint16_t machine;
        std::uint16_t num_sections;
        std::uint32_t timedate_stamp;
        std::uint32_t ptr_symbols;
        std::uint32_t num_symbols;
        std::uint16_t size_optional_header;
        std::uint16_t characteristics;
    };

    struct DosHeader final
    {
        std::uint16_t e_magic;
        std::uint16_t e_cblp;
        std::uint16_t e_cp;
        std::uint16_t e_crlc;
        std::uint16_t e_cparhdr;
        std::uint16_t e_minalloc;
        std::uint16_t e_maxalloc;
        std::uint16_t e_ss;
        std::uint16_t e_sp;
        std::uint16_t e_csum;
        std::uint16_t e_ip;
        std::uint16_t e_cs;
        std::uint16_t e_lfarlc;
        std::uint16_t e_ovno;
        std::uint16_t e_res[4];
        std::uint16_t e_oemid;
        std::uint16_t e_oeminfo;
        std::uint16_t e_res2[10];
        std::uint32_t e_lfanew;
    };

    enum class NtArchitecture
    {
        x32_bit,
        x64_bit,
    };
    template<NtArchitecture architecture>
    struct ImageNtHeaders final
    {
        std::uint32_t signature;
        FileHeader file_header;
        OptionalHeader<architecture == NtArchitecture::x64_bit> optional_header;
    };

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
        std::uint32_t num_relocs;
        std::uint32_t num_line_numbers;

        std::uint32_t characteristics;
    };
    // ReSharper restore CppDeclaratorNeverUsed

    using NtHeaderVariant =
            std::variant<ImageNtHeaders<NtArchitecture::x64_bit>, ImageNtHeaders<NtArchitecture::x32_bit>>;
} // namespace

// Internal PE scanner functions
namespace
{
    [[nodiscard]]
    std::optional<NtHeaderVariant> get_nt_header_from_file(std::fstream& file, const DosHeader& dos_header)
    {
        ImageNtHeaders<NtArchitecture::x32_bit> x86_headers{};
        file.seekg(dos_header.e_lfanew, std::ios::beg);
        file.read(reinterpret_cast<char*>(&x86_headers), sizeof(x86_headers));

        if (x86_headers.optional_header.magic == opt_hdr32_magic)
            return x86_headers;

        if (x86_headers.optional_header.magic != opt_hdr64_magic)
            return std::nullopt;

        ImageNtHeaders<NtArchitecture::x64_bit> x64_headers{};
        file.seekg(dos_header.e_lfanew, std::ios::beg);
        file.read(reinterpret_cast<char*>(&x64_headers), sizeof(x64_headers));

        return x64_headers;
    }

    [[nodiscard]]
    std::optional<NtHeaderVariant> get_nt_header_from_loaded_module(const void* module_base_address)
    {
        const auto module_byte_ptr = static_cast<const std::byte*>(module_base_address);
        ImageNtHeaders<NtArchitecture::x32_bit> x86_headers{};
        const auto dos_header = static_cast<const DosHeader*>(module_base_address);

        x86_headers = *reinterpret_cast<const ImageNtHeaders<NtArchitecture::x32_bit>*>(module_byte_ptr
                                                                                        + dos_header->e_lfanew);

        if (x86_headers.optional_header.magic == opt_hdr32_magic)
            return x86_headers;

        if (x86_headers.optional_header.magic != opt_hdr64_magic)
            return std::nullopt;

        return *reinterpret_cast<const ImageNtHeaders<NtArchitecture::x64_bit>*>(module_byte_ptr
                                                                                 + dos_header->e_lfanew);
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
        return std::visit([&nt_hdr_magic](const auto& header) -> bool { return header.signature != nt_hdr_magic; },
                          variant);
    }

    struct ExtractedSection
    {
        std::uint64_t virtual_base_addr;
        std::uint64_t raw_base_addr;
        std::vector<std::byte> data;
    };

    [[nodiscard]]
    std::optional<ExtractedSection> extract_section_from_pe_file(const std::filesystem::path& path_to_file,
                                                                 const std::string_view& section_name)
    {
        std::fstream file(path_to_file, std::ios::binary | std::ios::in);

        if (!file.is_open()) [[unlikely]]
            return std::nullopt;

        DosHeader dos_header{};
        file.read(reinterpret_cast<char*>(&dos_header), sizeof(dos_header));

        if (invalid_dos_header_file(dos_header)) [[unlikely]]
            return std::nullopt;

        const auto nt_headers = get_nt_header_from_file(file, dos_header);

        if (!nt_headers) [[unlikely]]
            return std::nullopt;

        if (invalid_nt_header_file(nt_headers.value())) [[unlikely]]
            return std::nullopt;

        return std::visit(
                [&file, &dos_header, &section_name](auto& concrete_headers) -> std::optional<ExtractedSection>
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
                        return ExtractedSection{.virtual_base_addr = current_section.virtual_address
                                                                     + concrete_headers.optional_header.image_base,
                                                .raw_base_addr = current_section.ptr_raw_data,
                                                .data = std::move(section_data)};
                    }
                    return std::nullopt;
                },
                nt_headers.value());
    }
} // namespace

namespace omath
{

    std::optional<std::uintptr_t>
    PePatternScanner::scan_for_pattern_in_loaded_module(const void* module_base_address,
                                                        const std::string_view& pattern,
                                                        const std::string_view& target_section_name)
    {
        const auto base_address = reinterpret_cast<std::uintptr_t>(module_base_address);
        const auto* base_bytes = static_cast<const std::byte*>(module_base_address);

        if (!base_address)
            return std::nullopt;

        const auto* dos_header = static_cast<const DosHeader*>(module_base_address);

        if (invalid_dos_header_file(*dos_header)) [[unlikely]]
            return std::nullopt;

        const auto nt_header_variant = get_nt_header_from_loaded_module(module_base_address);

        if (!nt_header_variant) [[unlikely]]
            return std::nullopt;

        return std::visit(
                [base_bytes, base_address, lfanew = dos_header->e_lfanew, &target_section_name,
                 &pattern](const auto& nt_header) -> std::optional<std::uintptr_t>
                {
                    constexpr std::size_t signature_size = sizeof(nt_header.signature);
                    const auto section_table_off = static_cast<std::size_t>(lfanew) + signature_size
                                                   + sizeof(FileHeader) + nt_header.file_header.size_optional_header;

                    const auto* section_table = reinterpret_cast<const SectionHeader*>(base_bytes + section_table_off);

                    for (std::size_t i = 0; i < nt_header.file_header.num_sections; ++i)
                    {
                        const auto* section = section_table + i;

                        if (std::string_view{section->name} != target_section_name || section->size_raw_data == 0)
                            continue;

                        const auto section_size = section->virtual_size != 0
                                                          ? static_cast<std::size_t>(section->virtual_size)
                                                          : static_cast<std::size_t>(section->size_raw_data);

                        const auto* section_begin =
                                reinterpret_cast<std::byte*>(base_address + section->virtual_address);
                        const auto scan_range = std::span{section_begin, section_size};

                        const auto result =
                                PatternScanner::scan_for_pattern(scan_range.begin(), scan_range.end(), pattern);

                        if (result != scan_range.end())
                            return reinterpret_cast<std::uintptr_t>(&*result);
                    }

                    return std::nullopt;
                },
                nt_header_variant.value());
    }
    std::optional<PeSectionScanResult>
    PePatternScanner::scan_for_pattern_in_file(const std::filesystem::path& path_to_file,
                                               const std::string_view& pattern,
                                               const std::string_view& target_section_name)
    {
        const auto pe_section = extract_section_from_pe_file(path_to_file, target_section_name);

        if (!pe_section.has_value()) [[unlikely]]
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
} // namespace omath