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
        using SectionHeader = std::conditional_t<arch == FILE_ARCH::x64, Elf32_Shdr, Elf32_Shdr>;
    };
    using ElfHeaderVariant = std::variant<Elf64_Ehdr, Elf32_Ehdr>;

    /*[[nodiscard]]
    std::optional<ElfHeaderVariant> get_elf_header_from_file(std::fstream& file)
    {
        return std::nullopt;
    }*/
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
    [[maybe_unused]]
    std::vector<uint8_t> get_elf_section_by_name(const std::string& path, const std::string& section_name)
    {
        std::fstream file(path, std::ios::binary | std::ios::in);

        if (!file.is_open())
            return {};

        if (not_elf_file(file))
            return {};

        const auto architecture = get_file_arch(file);

        if (!architecture.has_value())
            return {};

        auto read_bytes = [&](std::streamoff offset, char* buf, std::size_t sz) -> bool
        {
            file.seekg(offset, std::ios::beg);
            return file.good() && file.read(buf, sz);
        };

        std::variant<Elf32_Ehdr, Elf64_Ehdr> elf_header;

        if (architecture.value() == FILE_ARCH::x64)
            elf_header = Elf64_Ehdr{};
        else if (architecture.value() == FILE_ARCH::x32)
            elf_header = Elf32_Ehdr{};

        std::visit([&file](auto& header)
        {
            file.seekg(0, std::ios_base::beg);
            file.read(reinterpret_cast<char*>(&header), sizeof(header));
        }, elf_header);
        if (architecture.value() == FILE_ARCH::x64)
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
        [[maybe_unused]] auto _ = get_elf_section_by_name(path_to_file, ".text");

        return std::nullopt;
    }
} // namespace omath