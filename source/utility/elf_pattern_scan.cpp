//
// Created by Vladislav on 30.12.2025.
//
#include "omath/utility/pattern_scan.hpp"
#include <fstream>
#include <omath/utility/elf_pattern_scan.hpp>
#include <variant>
#include <vector>

#pragma pack(push, 1)

namespace
{
    // Common
    constexpr uint8_t EI_NIDENT = 16;
    constexpr uint8_t EI_CLASS = 4;

    constexpr uint8_t ELFCLASS32 = 1;
    constexpr uint8_t ELFCLASS64 = 2;

    struct Elf32_Ehdr
    {
        unsigned char e_ident[EI_NIDENT];
        uint16_t e_type;
        uint16_t e_machine;
        uint32_t e_version;
        uint32_t e_entry;
        uint32_t e_phoff;
        uint32_t e_shoff;
        uint32_t e_flags;
        uint16_t e_ehsize;
        uint16_t e_phentsize;
        uint16_t e_phnum;
        uint16_t e_shentsize;
        uint16_t e_shnum;
        uint16_t e_shstrndx;
    };

    struct Elf64_Ehdr
    {
        unsigned char e_ident[EI_NIDENT];
        uint16_t e_type;
        uint16_t e_machine;
        uint32_t e_version;
        uint64_t e_entry;
        uint64_t e_phoff;
        uint64_t e_shoff;
        uint32_t e_flags;
        uint16_t e_ehsize;
        uint16_t e_phentsize;
        uint16_t e_phnum;
        uint16_t e_shentsize;
        uint16_t e_shnum;
        uint16_t e_shstrndx;
    };

    struct Elf32_Shdr
    {
        uint32_t sh_name;
        uint32_t sh_type;
        uint32_t sh_flags;
        uint32_t sh_addr;
        uint32_t sh_offset;
        uint32_t sh_size;
        uint32_t sh_link;
        uint32_t sh_info;
        uint32_t sh_addralign;
        uint32_t sh_entsize;
    };

    struct Elf64_Shdr
    {
        uint32_t sh_name;
        uint32_t sh_type;
        uint64_t sh_flags;
        uint64_t sh_addr;
        uint64_t sh_offset;
        uint64_t sh_size;
        uint32_t sh_link;
        uint32_t sh_info;
        uint64_t sh_addralign;
        uint64_t sh_entsize;
    };

#pragma pack(pop)
} // namespace

namespace
{
    enum class FILE_ARCH : std::int8_t
    {
        x32,
        x64,
    };
    template<FILE_ARCH arch>
    struct ElfHeaders
    {
        using FileHeader = std::conditional_t<arch == FILE_ARCH::x64, Elf64_Ehdr, Elf32_Ehdr>;
        using SectionHeader = std::conditional_t<arch == FILE_ARCH::x64, Elf64_Shdr, Elf32_Shdr>;
        FileHeader file_header;
        SectionHeader section_header;
    };
    [[nodiscard]]
    bool not_elf_file(std::fstream& file)
    {
        constexpr std::string_view valid_elf_signature = "\x7F"
                                                         "ELF";
        std::array<char, valid_elf_signature.size() + 1> elf_signature{};
        const std::streampos back_up_pose = file.tellg();

        file.seekg(0, std::ios_base::beg);
        file.read(elf_signature.data(), 4);
        file.seekg(back_up_pose, std::ios_base::beg);

        return std::string_view{elf_signature.data(), 4} != valid_elf_signature;
    }
    [[nodiscard]]
    std::optional<FILE_ARCH> get_file_arch(std::fstream& file)
    {
        std::array<char, EI_NIDENT> e_ident{};
        const std::streampos back_up_pose = file.tellg();

        file.seekg(0, std::ios_base::beg);
        file.read(e_ident.data(), e_ident.size());
        file.seekg(back_up_pose, std::ios_base::beg);

        if (e_ident[EI_CLASS] == ELFCLASS64)
            return FILE_ARCH::x64;

        if (e_ident[EI_CLASS] == ELFCLASS32)
            return FILE_ARCH::x32;

        return std::nullopt;
    }
    struct ExtractedSection
    {
        std::uint64_t virtual_base_addr{};
        std::uint64_t raw_base_addr{};
        std::vector<std::byte> data;
    };
    [[maybe_unused]]
    std::optional<ExtractedSection> get_elf_section_by_name(const std::string& path,
                                                            const std::string_view& section_name)
    {
        std::fstream file(path, std::ios::binary | std::ios::in);

        if (!file.is_open())
            return std::nullopt;

        if (not_elf_file(file))
            return std::nullopt;

        const auto architecture = get_file_arch(file);

        if (!architecture.has_value())
            return std::nullopt;

        std::variant<ElfHeaders<FILE_ARCH::x64>, ElfHeaders<FILE_ARCH::x32>> elf_headers;
        if (architecture.value() == FILE_ARCH::x64)
            elf_headers = ElfHeaders<FILE_ARCH::x64>{};
        else if (architecture.value() == FILE_ARCH::x32)
            elf_headers = ElfHeaders<FILE_ARCH::x32>{};

        return std::visit(
                [&](auto& header) -> std::optional<ExtractedSection>
                {
                    auto& [file_header, section_header] = header;
                    file.seekg(0, std::ios_base::beg);
                    if (!file.read(reinterpret_cast<char*>(&file_header), sizeof(file_header)))
                        return std::nullopt;

                    std::streamoff shstr_off =
                            static_cast<std::streamoff>(file_header.e_shoff)
                            + static_cast<std::streamoff>(file_header.e_shstrndx) * sizeof(section_header);
                    file.seekg(shstr_off, std::ios_base::beg);

                    if (!file.read(reinterpret_cast<char*>(&section_header), sizeof(section_header)))
                        return std::nullopt;

                    std::vector<char> shstrtab(section_header.sh_size);

                    file.seekg(section_header.sh_offset, std::ios_base::beg);

                    if (!file.read(shstrtab.data(), shstrtab.size()))
                        return std::nullopt;

                    for (std::uint16_t i = 0; i < file_header.e_shnum; ++i)
                    {
                        decltype(section_header) current_section{};
                        const std::streamoff off = static_cast<std::streamoff>(file_header.e_shoff)
                                                   + static_cast<std::streamoff>(i) * sizeof(current_section);

                        file.seekg(off, std::ios_base::beg);
                        if (!file.read(reinterpret_cast<char*>(&current_section), sizeof(current_section)))
                            return std::nullopt;

                        if (current_section.sh_name >= shstrtab.size())
                            continue;

                        const std::string_view name = &shstrtab[current_section.sh_name];
                        if (section_name != name)
                            continue;

                        ExtractedSection out;

                        out.virtual_base_addr = current_section.sh_addr;
                        out.raw_base_addr = current_section.sh_offset;
                        out.data.resize(current_section.sh_size);

                        file.seekg(out.raw_base_addr, std::ios_base::beg);
                        if (!file.read(reinterpret_cast<char*>(out.data.data()), out.data.size()))
                            return std::nullopt;

                        return out;
                    }
                    return std::nullopt;
                },
                elf_headers);
    }
} // namespace
namespace omath
{

    std::optional<std::uintptr_t>
    ElfPatternScanner::scan_for_pattern_in_loaded_module([[maybe_unused]] const void* module_base_address,
                                                         [[maybe_unused]] const std::string_view& pattern)
    {
        return std::nullopt;
    }
    std::optional<ElfSectionScanResult>
    ElfPatternScanner::scan_for_pattern_in_file(const std::filesystem::path& path_to_file,
                                                const std::string_view& pattern,
                                                const std::string_view& target_section_name)
    {
        const auto pe_section = get_elf_section_by_name(path_to_file, target_section_name);

        if (!pe_section.has_value())
            return std::nullopt;

        const auto scan_result =
                PatternScanner::scan_for_pattern(pe_section->data.cbegin(), pe_section->data.cend(), pattern);

        if (scan_result == pe_section->data.cend())
            return std::nullopt;
        const auto offset = std::distance(pe_section->data.begin(), scan_result);

        return ElfSectionScanResult{.virtual_base_addr = pe_section->virtual_base_addr,
                                    .raw_base_addr = pe_section->raw_base_addr,
                                    .target_offset = offset};
    }
} // namespace omath