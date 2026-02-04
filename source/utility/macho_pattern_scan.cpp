//
// Created by Copilot on 04.02.2026.
//
#include "omath/utility/macho_pattern_scan.hpp"
#include "omath/utility/pattern_scan.hpp"
#include <cstring>
#include <fstream>
#include <variant>
#include <vector>

#pragma pack(push, 1)

namespace
{
    // Mach-O magic numbers
    constexpr std::uint32_t mh_magic_32 = 0xFEEDFACE;
    constexpr std::uint32_t mh_magic_64 = 0xFEEDFACF;
    constexpr std::uint32_t mh_cigam_32 = 0xCEFAEDFE; // Byte-swapped 32-bit
    constexpr std::uint32_t mh_cigam_64 = 0xCFFAEDFE; // Byte-swapped 64-bit

    // Load command types
    constexpr std::uint32_t lc_segment = 0x1;
    constexpr std::uint32_t lc_segment_64 = 0x19;

    // ReSharper disable CppDeclaratorNeverUsed
    // Mach-O header for 32-bit
    struct MachHeader32 final
    {
        std::uint32_t magic;
        std::uint32_t cputype;
        std::uint32_t cpusubtype;
        std::uint32_t filetype;
        std::uint32_t ncmds;
        std::uint32_t sizeofcmds;
        std::uint32_t flags;
    };

    // Mach-O header for 64-bit
    struct MachHeader64 final
    {
        std::uint32_t magic;
        std::uint32_t cputype;
        std::uint32_t cpusubtype;
        std::uint32_t filetype;
        std::uint32_t ncmds;
        std::uint32_t sizeofcmds;
        std::uint32_t flags;
        std::uint32_t reserved;
    };

    // Load command header
    struct LoadCommand final
    {
        std::uint32_t cmd;
        std::uint32_t cmdsize;
    };

    // Segment command for 32-bit
    struct SegmentCommand32 final
    {
        std::uint32_t cmd;
        std::uint32_t cmdsize;
        char segname[16];
        std::uint32_t vmaddr;
        std::uint32_t vmsize;
        std::uint32_t fileoff;
        std::uint32_t filesize;
        std::uint32_t maxprot;
        std::uint32_t initprot;
        std::uint32_t nsects;
        std::uint32_t flags;
    };

    // Segment command for 64-bit
    struct SegmentCommand64 final
    {
        std::uint32_t cmd;
        std::uint32_t cmdsize;
        char segname[16];
        std::uint64_t vmaddr;
        std::uint64_t vmsize;
        std::uint64_t fileoff;
        std::uint64_t filesize;
        std::uint32_t maxprot;
        std::uint32_t initprot;
        std::uint32_t nsects;
        std::uint32_t flags;
    };

    // Section for 32-bit
    struct Section32 final
    {
        char sectname[16];
        char segname[16];
        std::uint32_t addr;
        std::uint32_t size;
        std::uint32_t offset;
        std::uint32_t align;
        std::uint32_t reloff;
        std::uint32_t nreloc;
        std::uint32_t flags;
        std::uint32_t reserved1;
        std::uint32_t reserved2;
    };

    // Section for 64-bit
    struct Section64 final
    {
        char sectname[16];
        char segname[16];
        std::uint64_t addr;
        std::uint64_t size;
        std::uint32_t offset;
        std::uint32_t align;
        std::uint32_t reloff;
        std::uint32_t nreloc;
        std::uint32_t flags;
        std::uint32_t reserved1;
        std::uint32_t reserved2;
        std::uint32_t reserved3;
    };
    // ReSharper enable CppDeclaratorNeverUsed
#pragma pack(pop)

    enum class MachOArch : std::int8_t
    {
        x32,
        x64,
    };

    struct ExtractedSection final
    {
        std::uintptr_t virtual_base_addr{};
        std::uintptr_t raw_base_addr{};
        std::vector<std::byte> data;
    };

    [[nodiscard]]
    std::optional<MachOArch> get_macho_arch(std::fstream& file)
    {
        std::uint32_t magic{};
        const std::streampos backup_pos = file.tellg();

        file.seekg(0, std::ios_base::beg);
        file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
        file.seekg(backup_pos, std::ios_base::beg);

        if (magic == mh_magic_64 || magic == mh_cigam_64)
            return MachOArch::x64;

        if (magic == mh_magic_32 || magic == mh_cigam_32)
            return MachOArch::x32;

        return std::nullopt;
    }

    [[nodiscard]]
    bool is_macho_file(std::fstream& file)
    {
        return get_macho_arch(file).has_value();
    }

    [[nodiscard]]
    std::string_view get_section_name(const char* sectname)
    {
        // Mach-O section names are fixed 16-byte arrays, not necessarily null-terminated
        return std::string_view(sectname, std::min(std::strlen(sectname), std::size_t{16}));
    }

    template<typename HeaderType, typename SegmentType, typename SectionType, std::uint32_t segment_cmd>
    std::optional<ExtractedSection> extract_section_impl(std::fstream& file, const std::string_view& section_name)
    {
        HeaderType header{};
        file.seekg(0, std::ios_base::beg);
        if (!file.read(reinterpret_cast<char*>(&header), sizeof(header))) [[unlikely]]
            return std::nullopt;

        std::streamoff cmd_offset = sizeof(header);

        for (std::uint32_t i = 0; i < header.ncmds; ++i)
        {
            LoadCommand lc{};
            file.seekg(cmd_offset, std::ios_base::beg);
            if (!file.read(reinterpret_cast<char*>(&lc), sizeof(lc))) [[unlikely]]
                return std::nullopt;

            if (lc.cmd == segment_cmd)
            {
                SegmentType segment{};
                file.seekg(cmd_offset, std::ios_base::beg);
                if (!file.read(reinterpret_cast<char*>(&segment), sizeof(segment))) [[unlikely]]
                    return std::nullopt;

                std::streamoff sect_offset = cmd_offset + static_cast<std::streamoff>(sizeof(segment));

                for (std::uint32_t j = 0; j < segment.nsects; ++j)
                {
                    SectionType section{};
                    file.seekg(sect_offset, std::ios_base::beg);
                    if (!file.read(reinterpret_cast<char*>(&section), sizeof(section))) [[unlikely]]
                        return std::nullopt;

                    if (get_section_name(section.sectname) == section_name)
                    {
                        ExtractedSection out;
                        out.virtual_base_addr = static_cast<std::uintptr_t>(section.addr);
                        out.raw_base_addr = static_cast<std::uintptr_t>(section.offset);
                        out.data.resize(static_cast<std::size_t>(section.size));

                        file.seekg(static_cast<std::streamoff>(section.offset), std::ios_base::beg);
                        if (!file.read(reinterpret_cast<char*>(out.data.data()),
                                       static_cast<std::streamsize>(out.data.size()))) [[unlikely]]
                            return std::nullopt;

                        return out;
                    }

                    sect_offset += static_cast<std::streamoff>(sizeof(section));
                }
            }

            cmd_offset += static_cast<std::streamoff>(lc.cmdsize);
        }

        return std::nullopt;
    }

    [[nodiscard]]
    std::optional<ExtractedSection> get_macho_section_by_name(const std::filesystem::path& path,
                                                              const std::string_view& section_name)
    {
        std::fstream file(path, std::ios::binary | std::ios::in);

        if (!file.is_open()) [[unlikely]]
            return std::nullopt;

        if (!is_macho_file(file)) [[unlikely]]
            return std::nullopt;

        const auto arch = get_macho_arch(file);

        if (!arch.has_value()) [[unlikely]]
            return std::nullopt;

        if (arch.value() == MachOArch::x64)
            return extract_section_impl<MachHeader64, SegmentCommand64, Section64, lc_segment_64>(file, section_name);
        return extract_section_impl<MachHeader32, SegmentCommand32, Section32, lc_segment>(file, section_name);
    }

    template<typename HeaderType, typename SegmentType, typename SectionType, std::uint32_t segment_cmd>
    std::optional<std::uintptr_t> scan_in_module_impl(const std::byte* base, const std::string_view pattern,
                                                      const std::string_view target_section_name)
    {
        const auto* header = reinterpret_cast<const HeaderType*>(base);

        std::size_t cmd_offset = sizeof(HeaderType);

        for (std::uint32_t i = 0; i < header->ncmds; ++i)
        {
            const auto* lc = reinterpret_cast<const LoadCommand*>(base + cmd_offset);

            if (lc->cmd != segment_cmd)
            {
                cmd_offset += lc->cmdsize;
                continue;
            }
            const auto* segment = reinterpret_cast<const SegmentType*>(base + cmd_offset);
            std::size_t sect_offset = cmd_offset + sizeof(SegmentType);

            for (std::uint32_t j = 0; j < segment->nsects; ++j)
            {
                const auto* section = reinterpret_cast<const SectionType*>(base + sect_offset);

                if (get_section_name(section->sectname) != target_section_name && section->size > 0)
                {
                    sect_offset += sizeof(SectionType);
                    continue;
                }
                const auto* section_begin = base + static_cast<std::size_t>(section->addr);
                const auto* section_end = section_begin + static_cast<std::size_t>(section->size);

                const auto scan_result = omath::PatternScanner::scan_for_pattern(section_begin, section_end, pattern);

                if (scan_result != section_end)
                    return reinterpret_cast<std::uintptr_t>(scan_result);
            }
        }

        return std::nullopt;
    }

} // namespace

namespace omath
{
    std::optional<std::uintptr_t>
    MachOPatternScanner::scan_for_pattern_in_loaded_module(const void* module_base_address,
                                                           const std::string_view& pattern,
                                                           const std::string_view& target_section_name)
    {
        if (module_base_address == nullptr) [[unlikely]]
            return std::nullopt;

        const auto* base = static_cast<const std::byte*>(module_base_address);

        // Read magic to determine architecture
        std::uint32_t magic{};
        std::memcpy(&magic, base, sizeof(magic));

        if (magic == mh_magic_64 || magic == mh_cigam_64)
            return scan_in_module_impl<MachHeader64, SegmentCommand64, Section64, lc_segment_64>(base, pattern,
                                                                                                 target_section_name);

        if (magic == mh_magic_32 || magic == mh_cigam_32)
            return scan_in_module_impl<MachHeader32, SegmentCommand32, Section32, lc_segment>(base, pattern,
                                                                                              target_section_name);

        return std::nullopt;
    }

    std::optional<SectionScanResult>
    MachOPatternScanner::scan_for_pattern_in_file(const std::filesystem::path& path_to_file,
                                                  const std::string_view& pattern,
                                                  const std::string_view& target_section_name)
    {
        const auto macho_section = get_macho_section_by_name(path_to_file, target_section_name);

        if (!macho_section.has_value()) [[unlikely]]
            return std::nullopt;

        const auto scan_result =
                PatternScanner::scan_for_pattern(macho_section->data.cbegin(), macho_section->data.cend(), pattern);

        if (scan_result == macho_section->data.cend())
            return std::nullopt;

        const auto offset = std::distance(macho_section->data.begin(), scan_result);

        return SectionScanResult{.virtual_base_addr = macho_section->virtual_base_addr,
                                 .raw_base_addr = macho_section->raw_base_addr,
                                 .target_offset = offset};
    }
} // namespace omath
