//
// Created by Vladislav on 30.12.2025.
//
#include "omath/utility/pattern_scan.hpp"
#include <fstream>
#include <omath/utility/elf_pattern_scan.hpp>
#include <variant>
#include <vector>
#include <array>

#pragma pack(push, 1)

namespace
{
    // Common
    constexpr uint8_t ei_nident = 16;
    constexpr uint8_t ei_class = 4;

    constexpr uint8_t elfclass32 = 1;
    constexpr uint8_t elfclass64 = 2;
    // ReSharper disable CppDeclaratorNeverUsed
    struct Elf32Ehdr final
    {
        unsigned char e_ident[ei_nident];
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

    struct Elf64Ehdr final
    {
        unsigned char e_ident[ei_nident];
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

    struct Elf32Shdr final
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

    struct Elf64Shdr final
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
    // ReSharper restore CppDeclaratorNeverUsed
#pragma pack(pop)
} // namespace

namespace
{
    enum class FileArch : std::int8_t
    {
        x32,
        x64,
    };
    template<FileArch arch>
    struct ElfHeaders
    {
        using FileHeader = std::conditional_t<arch == FileArch::x64, Elf64Ehdr, Elf32Ehdr>;
        using SectionHeader = std::conditional_t<arch == FileArch::x64, Elf64Shdr, Elf32Shdr>;
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
    std::optional<FileArch> get_file_arch(std::fstream& file)
    {
        std::array<char, ei_nident> e_ident{};
        const std::streampos back_up_pose = file.tellg();

        file.seekg(0, std::ios_base::beg);
        file.read(e_ident.data(), e_ident.size());
        file.seekg(back_up_pose, std::ios_base::beg);

        if (e_ident[ei_class] == elfclass64)
            return FileArch::x64;

        if (e_ident[ei_class] == elfclass32)
            return FileArch::x32;

        return std::nullopt;
    }
    struct ExtractedSection final
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

        if (!file.is_open()) [[unlikely]]
            return std::nullopt;

        if (not_elf_file(file)) [[unlikely]]
            return std::nullopt;

        const auto architecture = get_file_arch(file);

        if (!architecture.has_value()) [[unlikely]]
            return std::nullopt;

        std::variant<ElfHeaders<FileArch::x64>, ElfHeaders<FileArch::x32>> elf_headers;
        if (architecture.value() == FileArch::x64)
            elf_headers = ElfHeaders<FileArch::x64>{};
        else if (architecture.value() == FileArch::x32)
            elf_headers = ElfHeaders<FileArch::x32>{};

        return std::visit(
                [&](auto& header) -> std::optional<ExtractedSection>
                {
                    auto& [file_header, section_header] = header;
                    file.seekg(0, std::ios_base::beg);
                    if (!file.read(reinterpret_cast<char*>(&file_header), sizeof(file_header))) [[unlikely]]
                        return std::nullopt;

                    const std::streamoff shstr_off =
                            static_cast<std::streamoff>(file_header.e_shoff)
                            + static_cast<std::streamoff>(file_header.e_shstrndx) * sizeof(section_header);
                    file.seekg(shstr_off, std::ios_base::beg);

                    if (!file.read(reinterpret_cast<char*>(&section_header), sizeof(section_header))) [[unlikely]]
                        return std::nullopt;

                    std::vector<char> shstrtab(section_header.sh_size);

                    file.seekg(section_header.sh_offset, std::ios_base::beg);

                    if (!file.read(shstrtab.data(), static_cast<std::streamsize>(shstrtab.size()))) [[unlikely]]
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

                        // ReSharper disable once CppTooWideScopeInitStatement
                        const std::string_view name = &shstrtab[current_section.sh_name];
                        if (section_name != name)
                            continue;

                        ExtractedSection out;

                        out.virtual_base_addr = current_section.sh_addr;
                        out.raw_base_addr = current_section.sh_offset;
                        out.data.resize(current_section.sh_size);

                        file.seekg(static_cast<std::streamoff>(out.raw_base_addr), std::ios_base::beg);
                        if (!file.read(reinterpret_cast<char*>(out.data.data()),
                                       static_cast<std::streamsize>(out.data.size()))) [[unlikely]]
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
    ElfPatternScanner::scan_for_pattern_in_loaded_module(const void* module_base_address,
                                                         const std::string_view& pattern,
                                                         const std::string_view& target_section_name)
    {
        if (module_base_address == nullptr)
            return std::nullopt;

        const auto* base = static_cast<const std::byte*>(module_base_address);

        // Validate ELF signature.
        constexpr std::string_view valid_elf_signature = "\x7F"
                                                         "ELF";
        if (std::string_view{reinterpret_cast<const char*>(base), valid_elf_signature.size()} != valid_elf_signature)
            return std::nullopt;

        // Detect architecture.
        const auto ei_class_value = static_cast<uint8_t>(base[ei_class]);
        const auto arch = ei_class_value == elfclass64   ? FileArch::x64
                          : ei_class_value == elfclass32 ? FileArch::x32
                                                         : std::optional<FileArch>{};
        if (!arch.has_value())
            return std::nullopt;

        auto scan_x64 = [&]() -> std::optional<std::uintptr_t>
        {
            using FileHeader = Elf64Ehdr;
            using SectionHeader = Elf64Shdr;

            const auto* file_header = reinterpret_cast<const FileHeader*>(base);

            const auto shstrtab_off = static_cast<std::size_t>(file_header->e_shoff)
                                      + static_cast<std::size_t>(file_header->e_shstrndx) * sizeof(SectionHeader);
            const auto* shstrtab_hdr = reinterpret_cast<const SectionHeader*>(base + shstrtab_off);
            const auto* shstrtab =
                    reinterpret_cast<const char*>(base + static_cast<std::size_t>(shstrtab_hdr->sh_offset));
            const auto shstrtab_size = static_cast<std::size_t>(shstrtab_hdr->sh_size);

            for (std::uint16_t i = 0; i < file_header->e_shnum; ++i)
            {
                const auto section_off = static_cast<std::size_t>(file_header->e_shoff)
                                         + static_cast<std::size_t>(i) * sizeof(SectionHeader);
                const auto* section = reinterpret_cast<const SectionHeader*>(base + section_off);

                if (section->sh_name >= shstrtab_size || section->sh_size == 0)
                    continue;

                if (std::string_view{shstrtab + section->sh_name} != target_section_name)
                    continue;

                const auto* section_begin = base + static_cast<std::size_t>(section->sh_addr);
                const auto* section_end = section_begin + static_cast<std::size_t>(section->sh_size);

                const auto scan_result = PatternScanner::scan_for_pattern(section_begin, section_end, pattern);

                if (scan_result == section_end)
                    return std::nullopt;

                return reinterpret_cast<std::uintptr_t>(scan_result);
            }

            return std::nullopt;
        };

        auto scan_x32 = [&]() -> std::optional<std::uintptr_t>
        {
            using FileHeader = Elf32Ehdr;
            using SectionHeader = Elf32Shdr;

            const auto* file_header = reinterpret_cast<const FileHeader*>(base);

            const auto shstrtab_off = static_cast<std::size_t>(file_header->e_shoff)
                                      + static_cast<std::size_t>(file_header->e_shstrndx) * sizeof(SectionHeader);
            const auto* shstrtab_hdr = reinterpret_cast<const SectionHeader*>(base + shstrtab_off);
            const auto* shstrtab =
                    reinterpret_cast<const char*>(base + static_cast<std::size_t>(shstrtab_hdr->sh_offset));
            const auto shstrtab_size = static_cast<std::size_t>(shstrtab_hdr->sh_size);

            for (std::uint16_t i = 0; i < file_header->e_shnum; ++i)
            {
                const auto section_off = static_cast<std::size_t>(file_header->e_shoff)
                                         + static_cast<std::size_t>(i) * sizeof(SectionHeader);
                const auto* section = reinterpret_cast<const SectionHeader*>(base + section_off);

                if (section->sh_name >= shstrtab_size || section->sh_size == 0)
                    continue;

                const std::string_view name{shstrtab + section->sh_name};
                if (name != target_section_name)
                    continue;

                const auto* section_begin = base + static_cast<std::size_t>(section->sh_addr);
                const auto* section_end = section_begin + static_cast<std::size_t>(section->sh_size);

                const auto scan_result = PatternScanner::scan_for_pattern(section_begin, section_end, pattern);

                if (scan_result == section_end)
                    return std::nullopt;

                return reinterpret_cast<std::uintptr_t>(scan_result);
            }

            return std::nullopt;
        };

        if (arch == FileArch::x64)
            return scan_x64();

        return scan_x32();
    }
    std::optional<ElfSectionScanResult>
    ElfPatternScanner::scan_for_pattern_in_file(const std::filesystem::path& path_to_file,
                                                const std::string_view& pattern,
                                                const std::string_view& target_section_name)
    {
        const auto pe_section = get_elf_section_by_name(path_to_file, target_section_name);

        if (!pe_section.has_value()) [[unlikely]]
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