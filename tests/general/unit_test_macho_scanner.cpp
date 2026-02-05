//
// Created by Copilot on 04.02.2026.
//
// Unit tests for MachOPatternScanner
#include <gtest/gtest.h>
#include <omath/utility/macho_pattern_scan.hpp>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <vector>

using namespace omath;

namespace
{
    // Mach-O magic numbers
    constexpr std::uint32_t mh_magic_64 = 0xFEEDFACF;
    constexpr std::uint32_t mh_magic_32 = 0xFEEDFACE;
    constexpr std::uint32_t lc_segment = 0x1;
    constexpr std::uint32_t lc_segment_64 = 0x19;

    constexpr std::string_view segment_name = "__TEXT";
    constexpr std::string_view section_name = "__text";
#pragma pack(push, 1)
    struct MachHeader64
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

    struct MachHeader32
    {
        std::uint32_t magic;
        std::uint32_t cputype;
        std::uint32_t cpusubtype;
        std::uint32_t filetype;
        std::uint32_t ncmds;
        std::uint32_t sizeofcmds;
        std::uint32_t flags;
    };

    struct SegmentCommand64
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

    struct SegmentCommand32
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

    struct Section64
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

    struct Section32
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
#pragma pack(pop)

    // Helper function to create a minimal 64-bit Mach-O file with a __text section
    bool write_minimal_macho64_file(const std::string& path, const std::vector<std::uint8_t>& section_bytes)
    {
        std::ofstream f(path, std::ios::binary);
        if (!f.is_open())
            return false;

        // Calculate sizes
        constexpr std::size_t header_size = sizeof(MachHeader64);
        constexpr std::size_t segment_size = sizeof(SegmentCommand64);
        constexpr std::size_t section_size = sizeof(Section64);
        constexpr std::size_t load_cmd_size = segment_size + section_size;
        // Section data will start after headers
        const std::size_t section_offset = header_size + load_cmd_size;

        // Create Mach-O header
        MachHeader64 header{};
        header.magic = mh_magic_64;
        header.cputype = 0x01000007;  // CPU_TYPE_X86_64
        header.cpusubtype = 0x3;       // CPU_SUBTYPE_X86_64_ALL
        header.filetype = 0x2;         // MH_EXECUTE
        header.ncmds = 1;
        header.sizeofcmds = static_cast<std::uint32_t>(load_cmd_size);
        header.flags = 0;
        header.reserved = 0;

        f.write(reinterpret_cast<const char*>(&header), sizeof(header));

        // Create segment command
        SegmentCommand64 segment{};
        segment.cmd = lc_segment_64;
        segment.cmdsize = static_cast<std::uint32_t>(load_cmd_size);
        std::ranges::copy(segment_name, segment.segname);
        segment.vmaddr = 0x100000000;
        segment.vmsize = section_bytes.size();
        segment.fileoff = section_offset;
        segment.filesize = section_bytes.size();
        segment.maxprot = 7;   // VM_PROT_ALL
        segment.initprot = 5;  // VM_PROT_READ | VM_PROT_EXECUTE
        segment.nsects = 1;
        segment.flags = 0;

        f.write(reinterpret_cast<const char*>(&segment), sizeof(segment));

        // Create section
        Section64 section{};
        std::ranges::copy(section_name, section.sectname);
        std::ranges::copy(segment_name, segment.segname);
        section.addr = 0x100000000;
        section.size = section_bytes.size();
        section.offset = static_cast<std::uint32_t>(section_offset);
        section.align = 0;
        section.reloff = 0;
        section.nreloc = 0;
        section.flags = 0;
        section.reserved1 = 0;
        section.reserved2 = 0;
        section.reserved3 = 0;

        f.write(reinterpret_cast<const char*>(&section), sizeof(section));

        // Write section data
        f.write(reinterpret_cast<const char*>(section_bytes.data()), static_cast<std::streamsize>(section_bytes.size()));

        f.close();
        return true;
    }

    // Helper function to create a minimal 32-bit Mach-O file with a __text section
    bool write_minimal_macho32_file(const std::string& path, const std::vector<std::uint8_t>& section_bytes)
    {
        std::ofstream f(path, std::ios::binary);
        if (!f.is_open())
            return false;

        // Calculate sizes
        constexpr std::size_t header_size = sizeof(MachHeader32);
        constexpr std::size_t segment_size = sizeof(SegmentCommand32);
        constexpr std::size_t section_size = sizeof(Section32);
        constexpr std::size_t load_cmd_size = segment_size + section_size;

        // Section data will start after headers
        constexpr std::size_t section_offset = header_size + load_cmd_size;

        // Create Mach-O header
        MachHeader32 header{};
        header.magic = mh_magic_32;
        header.cputype = 0x7;          // CPU_TYPE_X86
        header.cpusubtype = 0x3;       // CPU_SUBTYPE_X86_ALL
        header.filetype = 0x2;         // MH_EXECUTE
        header.ncmds = 1;
        header.sizeofcmds = static_cast<std::uint32_t>(load_cmd_size);
        header.flags = 0;

        f.write(reinterpret_cast<const char*>(&header), sizeof(header));

        // Create segment command
        SegmentCommand32 segment{};
        segment.cmd = lc_segment;
        segment.cmdsize = static_cast<std::uint32_t>(load_cmd_size);
        std::ranges::copy(segment_name, segment.segname);
        segment.vmaddr = 0x1000;
        segment.vmsize = static_cast<std::uint32_t>(section_bytes.size());
        segment.fileoff = static_cast<std::uint32_t>(section_offset);
        segment.filesize = static_cast<std::uint32_t>(section_bytes.size());
        segment.maxprot = 7;   // VM_PROT_ALL
        segment.initprot = 5;  // VM_PROT_READ | VM_PROT_EXECUTE
        segment.nsects = 1;
        segment.flags = 0;

        f.write(reinterpret_cast<const char*>(&segment), sizeof(segment));

        // Create section
        Section32 section{};
        std::ranges::copy(section_name, section.sectname);
        std::ranges::copy(segment_name, segment.segname);
        section.addr = 0x1000;
        section.size = static_cast<std::uint32_t>(section_bytes.size());
        section.offset = static_cast<std::uint32_t>(section_offset);
        section.align = 0;
        section.reloff = 0;
        section.nreloc = 0;
        section.flags = 0;
        section.reserved1 = 0;
        section.reserved2 = 0;

        f.write(reinterpret_cast<const char*>(&section), sizeof(section));

        // Write section data
        f.write(reinterpret_cast<const char*>(section_bytes.data()), static_cast<std::streamsize>(section_bytes.size()));

        f.close();
        return true;
    }

} // namespace

// Test scanning for a pattern that exists in a 64-bit Mach-O file
TEST(unit_test_macho_pattern_scan_file, ScanFindsPattern64)
{
    constexpr std::string_view path = "./test_minimal_macho64.bin";
    const std::vector<std::uint8_t> bytes = {0x55, 0x48, 0x89, 0xE5, 0x90, 0x90}; // push rbp; mov rbp, rsp; nop; nop
    ASSERT_TRUE(write_minimal_macho64_file(path.data(), bytes));

    const auto res = MachOPatternScanner::scan_for_pattern_in_file(path, "55 48 89 E5", "__text");
    EXPECT_TRUE(res.has_value());
    if (res.has_value())
    {
        EXPECT_EQ(res->target_offset, 0);
    }
}

// Test scanning for a pattern that exists in a 32-bit Mach-O file
TEST(unit_test_macho_pattern_scan_file, ScanFindsPattern32)
{
    constexpr std::string_view path = "./test_minimal_macho32.bin";
    const std::vector<std::uint8_t> bytes = {0x55, 0x89, 0xE5, 0x90, 0x90}; // push ebp; mov ebp, esp; nop; nop
    ASSERT_TRUE(write_minimal_macho32_file(path.data(), bytes));

    const auto res = MachOPatternScanner::scan_for_pattern_in_file(path, "55 89 E5", "__text");
    EXPECT_TRUE(res.has_value());
    if (res.has_value())
    {
        EXPECT_EQ(res->target_offset, 0);
    }
}

// Test scanning for a pattern that does not exist
TEST(unit_test_macho_pattern_scan_file, ScanMissingPattern)
{
    constexpr std::string_view path = "./test_minimal_macho_missing.bin";
    const std::vector<std::uint8_t> bytes = {0x00, 0x01, 0x02, 0x03};
    ASSERT_TRUE(write_minimal_macho64_file(path.data(), bytes));

    const auto res = MachOPatternScanner::scan_for_pattern_in_file(path, "FF EE DD", "__text");
    EXPECT_FALSE(res.has_value());
}

// Test scanning for a pattern at a non-zero offset
TEST(unit_test_macho_pattern_scan_file, ScanPatternAtOffset)
{
    constexpr std::string_view path = "./test_minimal_macho_offset.bin";
    const std::vector<std::uint8_t> bytes = {0x90, 0x90, 0x90, 0x55, 0x48, 0x89, 0xE5}; // nops then pattern
    ASSERT_TRUE(write_minimal_macho64_file(path.data(), bytes));

    const auto res = MachOPatternScanner::scan_for_pattern_in_file(path, "55 48 89 E5", "__text");
    EXPECT_TRUE(res.has_value());
    if (res.has_value())
    {
        EXPECT_EQ(res->target_offset, 3);
    }
}

// Test scanning with wildcards
TEST(unit_test_macho_pattern_scan_file, ScanWithWildcard)
{
    constexpr std::string_view path = "./test_minimal_macho_wildcard.bin";
    const std::vector<std::uint8_t> bytes = {0x55, 0x48, 0x89, 0xE5, 0x90};
    ASSERT_TRUE(write_minimal_macho64_file(path.data(), bytes));

    const auto res = MachOPatternScanner::scan_for_pattern_in_file(path, "55 ? 89 E5", "__text");
    EXPECT_TRUE(res.has_value());
}

// Test scanning a non-existent file
TEST(unit_test_macho_pattern_scan_file, ScanNonExistentFile)
{
    const auto res = MachOPatternScanner::scan_for_pattern_in_file("/non/existent/file.bin", "55 48", "__text");
    EXPECT_FALSE(res.has_value());
}

// Test scanning an invalid (non-Mach-O) file
TEST(unit_test_macho_pattern_scan_file, ScanInvalidFile)
{
    constexpr std::string_view path = "./test_invalid_macho.bin";
    std::ofstream f(path.data(), std::ios::binary);
    const std::vector<std::uint8_t> garbage = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
    f.write(reinterpret_cast<const char*>(garbage.data()), static_cast<std::streamsize>(garbage.size()));
    f.close();

    const auto res = MachOPatternScanner::scan_for_pattern_in_file(path, "55 48", "__text");
    EXPECT_FALSE(res.has_value());
}

// Test scanning for a non-existent section
TEST(unit_test_macho_pattern_scan_file, ScanNonExistentSection)
{
    constexpr std::string_view path = "./test_minimal_macho_nosect.bin";
    const std::vector<std::uint8_t> bytes = {0x55, 0x48, 0x89, 0xE5};
    ASSERT_TRUE(write_minimal_macho64_file(path.data(), bytes));

    const auto res = MachOPatternScanner::scan_for_pattern_in_file(path, "55 48", "__nonexistent");
    EXPECT_FALSE(res.has_value());
}

// Test scanning with null module base address
TEST(unit_test_macho_pattern_scan_loaded, ScanNullModule)
{
    const auto res = MachOPatternScanner::scan_for_pattern_in_loaded_module(nullptr, "55 48", "__text");
    EXPECT_FALSE(res.has_value());
}

// Test scanning in loaded module with invalid magic
TEST(unit_test_macho_pattern_scan_loaded, ScanInvalidMagic)
{
    std::vector<std::uint8_t> invalid_data(256, 0x00);
    const auto res = MachOPatternScanner::scan_for_pattern_in_loaded_module(invalid_data.data(), "55 48", "__text");
    EXPECT_FALSE(res.has_value());
}
