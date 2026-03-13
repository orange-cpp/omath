//
// Created by Copilot on 04.02.2026.
//
// Unit tests for MachOPatternScanner
#include <gtest/gtest.h>
#include <omath/utility/macho_pattern_scan.hpp>
#include <cstdint>
#include <cstring>
#include <vector>
#include "mem_fd_helper.hpp"

using namespace omath;

namespace
{
    // Mach-O magic numbers
    constexpr std::uint32_t mh_magic_64 = 0xFEEDFACF;
    constexpr std::uint32_t mh_magic_32 = 0xFEEDFACE;
    constexpr std::uint32_t lc_segment  = 0x1;
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

    // Build a minimal 64-bit Mach-O binary in-memory with a __text section
    std::vector<std::uint8_t> build_minimal_macho64(const std::vector<std::uint8_t>& section_bytes)
    {
        constexpr std::size_t load_cmd_size = sizeof(SegmentCommand64) + sizeof(Section64);
        const std::size_t section_offset    = sizeof(MachHeader64) + load_cmd_size;

        std::vector<std::uint8_t> buf(section_offset + section_bytes.size(), 0u);

        auto* header = reinterpret_cast<MachHeader64*>(buf.data());
        header->magic      = mh_magic_64;
        header->cputype    = 0x01000007;  // CPU_TYPE_X86_64
        header->cpusubtype = 0x3;
        header->filetype   = 0x2;         // MH_EXECUTE
        header->ncmds      = 1;
        header->sizeofcmds = static_cast<std::uint32_t>(load_cmd_size);

        auto* segment = reinterpret_cast<SegmentCommand64*>(buf.data() + sizeof(MachHeader64));
        segment->cmd      = lc_segment_64;
        segment->cmdsize  = static_cast<std::uint32_t>(load_cmd_size);
        std::ranges::copy(segment_name, segment->segname);
        segment->vmaddr   = 0x100000000;
        segment->vmsize   = section_bytes.size();
        segment->fileoff  = section_offset;
        segment->filesize = section_bytes.size();
        segment->maxprot  = 7;
        segment->initprot = 5;
        segment->nsects   = 1;

        auto* section = reinterpret_cast<Section64*>(buf.data() + sizeof(MachHeader64) + sizeof(SegmentCommand64));
        std::ranges::copy(section_name, section->sectname);
        std::ranges::copy(segment_name, section->segname);
        section->addr   = 0x100000000;
        section->size   = section_bytes.size();
        section->offset = static_cast<std::uint32_t>(section_offset);

        std::memcpy(buf.data() + section_offset, section_bytes.data(), section_bytes.size());
        return buf;
    }

    // Build a minimal 32-bit Mach-O binary in-memory with a __text section
    std::vector<std::uint8_t> build_minimal_macho32(const std::vector<std::uint8_t>& section_bytes)
    {
        constexpr std::size_t load_cmd_size = sizeof(SegmentCommand32) + sizeof(Section32);
        constexpr std::size_t section_offset = sizeof(MachHeader32) + load_cmd_size;

        std::vector<std::uint8_t> buf(section_offset + section_bytes.size(), 0u);

        auto* header = reinterpret_cast<MachHeader32*>(buf.data());
        header->magic      = mh_magic_32;
        header->cputype    = 0x7;
        header->cpusubtype = 0x3;
        header->filetype   = 0x2;
        header->ncmds      = 1;
        header->sizeofcmds = static_cast<std::uint32_t>(load_cmd_size);

        auto* segment = reinterpret_cast<SegmentCommand32*>(buf.data() + sizeof(MachHeader32));
        segment->cmd      = lc_segment;
        segment->cmdsize  = static_cast<std::uint32_t>(load_cmd_size);
        std::ranges::copy(segment_name, segment->segname);
        segment->vmaddr   = 0x1000;
        segment->vmsize   = static_cast<std::uint32_t>(section_bytes.size());
        segment->fileoff  = static_cast<std::uint32_t>(section_offset);
        segment->filesize = static_cast<std::uint32_t>(section_bytes.size());
        segment->maxprot  = 7;
        segment->initprot = 5;
        segment->nsects   = 1;

        auto* section = reinterpret_cast<Section32*>(buf.data() + sizeof(MachHeader32) + sizeof(SegmentCommand32));
        std::ranges::copy(section_name, section->sectname);
        std::ranges::copy(segment_name, section->segname);
        section->addr   = 0x1000;
        section->size   = static_cast<std::uint32_t>(section_bytes.size());
        section->offset = static_cast<std::uint32_t>(section_offset);

        std::memcpy(buf.data() + section_offset, section_bytes.data(), section_bytes.size());
        return buf;
    }

} // namespace

TEST(unit_test_macho_pattern_scan_file, ScanFindsPattern64)
{
    const std::vector<std::uint8_t> bytes = {0x55, 0x48, 0x89, 0xE5, 0x90, 0x90};
    const auto f = MemFdFile::create(build_minimal_macho64(bytes));
    ASSERT_TRUE(f.valid());

    const auto res = MachOPatternScanner::scan_for_pattern_in_file(f.path(), "55 48 89 E5", "__text");
    EXPECT_TRUE(res.has_value());
    if (res.has_value())
        EXPECT_EQ(res->target_offset, 0);
}

TEST(unit_test_macho_pattern_scan_file, ScanFindsPattern32)
{
    const std::vector<std::uint8_t> bytes = {0x55, 0x89, 0xE5, 0x90, 0x90};
    const auto f = MemFdFile::create(build_minimal_macho32(bytes));
    ASSERT_TRUE(f.valid());

    const auto res = MachOPatternScanner::scan_for_pattern_in_file(f.path(), "55 89 E5", "__text");
    EXPECT_TRUE(res.has_value());
    if (res.has_value())
        EXPECT_EQ(res->target_offset, 0);
}

TEST(unit_test_macho_pattern_scan_file, ScanMissingPattern)
{
    const std::vector<std::uint8_t> bytes = {0x00, 0x01, 0x02, 0x03};
    const auto f = MemFdFile::create(build_minimal_macho64(bytes));
    ASSERT_TRUE(f.valid());

    const auto res = MachOPatternScanner::scan_for_pattern_in_file(f.path(), "FF EE DD", "__text");
    EXPECT_FALSE(res.has_value());
}

TEST(unit_test_macho_pattern_scan_file, ScanPatternAtOffset)
{
    const std::vector<std::uint8_t> bytes = {0x90, 0x90, 0x90, 0x55, 0x48, 0x89, 0xE5};
    const auto f = MemFdFile::create(build_minimal_macho64(bytes));
    ASSERT_TRUE(f.valid());

    const auto res = MachOPatternScanner::scan_for_pattern_in_file(f.path(), "55 48 89 E5", "__text");
    EXPECT_TRUE(res.has_value());
    if (res.has_value())
        EXPECT_EQ(res->target_offset, 3);
}

TEST(unit_test_macho_pattern_scan_file, ScanWithWildcard)
{
    const std::vector<std::uint8_t> bytes = {0x55, 0x48, 0x89, 0xE5, 0x90};
    const auto f = MemFdFile::create(build_minimal_macho64(bytes));
    ASSERT_TRUE(f.valid());

    const auto res = MachOPatternScanner::scan_for_pattern_in_file(f.path(), "55 ? 89 E5", "__text");
    EXPECT_TRUE(res.has_value());
}

TEST(unit_test_macho_pattern_scan_file, ScanNonExistentFile)
{
    const auto res = MachOPatternScanner::scan_for_pattern_in_file("/non/existent/file.bin", "55 48", "__text");
    EXPECT_FALSE(res.has_value());
}

TEST(unit_test_macho_pattern_scan_file, ScanInvalidFile)
{
    const std::vector<std::uint8_t> garbage = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
    const auto f = MemFdFile::create(garbage);
    ASSERT_TRUE(f.valid());

    const auto res = MachOPatternScanner::scan_for_pattern_in_file(f.path(), "55 48", "__text");
    EXPECT_FALSE(res.has_value());
}

TEST(unit_test_macho_pattern_scan_file, ScanNonExistentSection)
{
    const std::vector<std::uint8_t> bytes = {0x55, 0x48, 0x89, 0xE5};
    const auto f = MemFdFile::create(build_minimal_macho64(bytes));
    ASSERT_TRUE(f.valid());

    const auto res = MachOPatternScanner::scan_for_pattern_in_file(f.path(), "55 48", "__nonexistent");
    EXPECT_FALSE(res.has_value());
}

TEST(unit_test_macho_pattern_scan_loaded, ScanNullModule)
{
    const auto res = MachOPatternScanner::scan_for_pattern_in_loaded_module(nullptr, "55 48", "__text");
    EXPECT_FALSE(res.has_value());
}

TEST(unit_test_macho_pattern_scan_loaded, ScanInvalidMagic)
{
    std::vector<std::uint8_t> invalid_data(256, 0x00);
    const auto res = MachOPatternScanner::scan_for_pattern_in_loaded_module(invalid_data.data(), "55 48", "__text");
    EXPECT_FALSE(res.has_value());
}
