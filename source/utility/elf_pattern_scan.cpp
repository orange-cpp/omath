//
// Created by Vladislav on 30.12.2025.
//
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
    constexpr uint8_t EI_DATA = 5;

    //constexpr uint8_t ELFCLASS32 = 1;
    constexpr uint8_t ELFCLASS64 = 2;
    constexpr uint8_t ELFDATA2LSB = 1;

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
    using ElfHeaderVariant = std::variant<Elf64_Ehdr, Elf32_Ehdr>;

    /*[[nodiscard]]
    std::optional<ElfHeaderVariant> get_elf_header_from_file(std::fstream& file)
    {
        return std::nullopt;
    }*/
    [[nodiscard]]
    bool not_elf_file(std::fstream& file)
    {
        std::array<char, EI_NIDENT+1> elf_signature{};
        const std::streampos back_up_pose = file.tellg();
        file.seekg(0, std::ios_base::beg);
        file.read(elf_signature.data(), EI_NIDENT);
        file.seekg(back_up_pose, std::ios_base::beg);

        return std::string_view{elf_signature.data(), 4} !=  "\x7F" "ELF";
    }
    [[maybe_unused]]
    std::vector<uint8_t> GetElfSectionByName(const std::string& path, const std::string& section_name)
    {
        std::fstream f(path, std::ios::binary);
        if (!f)
            return {};

        auto read_bytes = [&](std::streamoff offset, char* buf, std::size_t sz) -> bool
        {
            f.seekg(offset, std::ios::beg);
            return f.good() && f.read(buf, sz);
        };

        // Read ident
        unsigned char e_ident[EI_NIDENT] = {};

        if (!not_elf_file(f))
            return {};


        bool is_64 = (e_ident[EI_CLASS] == ELFCLASS64);
        bool is_le = (e_ident[EI_DATA] == ELFDATA2LSB);
        if (!is_le)
            return {}; // Only little-endian supported here

        if (is_64)
        {
            Elf64_Ehdr eh{};
            if (!read_bytes(0, reinterpret_cast<char*>(&eh), sizeof(eh)))
                return {};

            // Section header string table header
            Elf64_Shdr shstr{};
            std::streamoff shstr_off = static_cast<std::streamoff>(eh.e_shoff)
                                       + static_cast<std::streamoff>(eh.e_shstrndx) * sizeof(Elf64_Shdr);
            if (!read_bytes(shstr_off, reinterpret_cast<char*>(&shstr), sizeof(shstr)))
                return {};

            // Read shstrtab
            std::vector<char> shstrtab(shstr.sh_size);
            if (!read_bytes(shstr.sh_offset, shstrtab.data(), shstrtab.size()))
                return {};

            for (uint16_t i = 0; i < eh.e_shnum; ++i)
            {
                Elf64_Shdr sh{};
                std::streamoff off =
                        static_cast<std::streamoff>(eh.e_shoff) + static_cast<std::streamoff>(i) * sizeof(Elf64_Shdr);
                if (!read_bytes(off, reinterpret_cast<char*>(&sh), sizeof(sh)))
                    return {};
                const char* name = (sh.sh_name < shstrtab.size()) ? &shstrtab[sh.sh_name] : nullptr;
                if (!name)
                    continue;
                if (section_name == name)
                {
                    std::vector<uint8_t> data(sh.sh_size);
                    if (!read_bytes(sh.sh_offset, reinterpret_cast<char*>(data.data()), data.size()))
                        return {};
                    return data;
                }
            }
        }
        else
        { // 32-bit
            Elf32_Ehdr eh{};
            if (!read_bytes(0, reinterpret_cast<char*>(&eh), sizeof(eh)))
                return {};

            Elf32_Shdr shstr{};
            std::streamoff shstr_off = static_cast<std::streamoff>(eh.e_shoff)
                                       + static_cast<std::streamoff>(eh.e_shstrndx) * sizeof(Elf32_Shdr);
            if (!read_bytes(shstr_off, reinterpret_cast<char*>(&shstr), sizeof(shstr)))
                return {};

            std::vector<char> shstrtab(shstr.sh_size);
            if (!read_bytes(shstr.sh_offset, shstrtab.data(), shstrtab.size()))
                return {};

            for (uint16_t i = 0; i < eh.e_shnum; ++i)
            {
                Elf32_Shdr sh{};
                std::streamoff off =
                        static_cast<std::streamoff>(eh.e_shoff) + static_cast<std::streamoff>(i) * sizeof(Elf32_Shdr);
                if (!read_bytes(off, reinterpret_cast<char*>(&sh), sizeof(sh)))
                    return {};
                const char* name = (sh.sh_name < shstrtab.size()) ? &shstrtab[sh.sh_name] : nullptr;
                if (!name)
                    continue;
                if (section_name == name)
                {
                    std::vector<uint8_t> data(sh.sh_size);
                    if (!read_bytes(sh.sh_offset, reinterpret_cast<char*>(data.data()), data.size()))
                        return {};
                    return data;
                }
            }
        }

        return {}; // Not found or error
    }
} // namespace
namespace omath
{

    std::optional<ElfSectionScanResult>
    ElfPatternScanner::scan_for_pattern_in_file(const std::filesystem::path& path_to_file,
                                                [[maybe_unused]] const std::string_view& pattern,
                                                [[maybe_unused]] const std::string_view& target_section_name)
    {
        std::fstream file(path_to_file, std::ios::binary | std::ios::in);

        if (!file.is_open()) [[unlikely]]
            return std::nullopt;

        [[maybe_unused]] bool flag = not_elf_file(file);
        return std::nullopt;
    }
} // namespace omath