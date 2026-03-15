// Tests for MachOPatternScanner::scan_for_pattern_in_memory_file
#include <cstring>
#include <gtest/gtest.h>
#include <omath/utility/macho_pattern_scan.hpp>
#include <span>
#include <vector>

using namespace omath;

// Build a minimal Mach-O 64-bit file in memory with a single __text section.
// Layout:
//   0x000 : MachHeader64        (32 bytes)
//   0x020 : SegmentCommand64    (72 bytes)
//   0x068 : Section64           (80 bytes)  ← follows segment command inline
//   0x0B8 : section raw data    (padded to 0x20 bytes)
static std::vector<std::byte> make_macho64_with_text_section(const std::vector<std::uint8_t>& code_bytes)
{
    constexpr std::uint32_t mh_magic_64 = 0xFEEDFACF;
    constexpr std::uint32_t lc_segment_64 = 0x19;

    // MachHeader64 layout (32 bytes):
    //  0 magic, 4 cputype, 8 cpusubtype, 12 filetype, 16 ncmds, 20 sizeofcmds, 24 flags, 28 reserved
    constexpr std::size_t hdr_size = 32;

    // SegmentCommand64 layout (72 bytes):
    //  0 cmd, 4 cmdsize, 8 segname[16], 24 vmaddr, 32 vmsize, 40 fileoff, 48 filesize,
    //  56 maxprot, 60 initprot, 64 nsects, 68 flags
    constexpr std::size_t seg_size = 72;

    // Section64 layout (80 bytes):
    //  0 sectname[16], 16 segname[16], 32 addr, 40 size, 48 offset, 52 align,
    //  56 reloff, 60 nreloc, 64 flags, 68 reserved1, 72 reserved2, 76 reserved3
    constexpr std::size_t sect_hdr_size = 80;

    constexpr std::size_t text_raw_off = hdr_size + seg_size + sect_hdr_size; // 0xB8
    constexpr std::size_t text_raw_size = 0x20;
    constexpr std::size_t total_size = text_raw_off + text_raw_size;
    constexpr std::uint64_t text_vmaddr = 0x100001000ULL;

    constexpr std::uint32_t cmd_size =
            static_cast<std::uint32_t>(seg_size + sect_hdr_size); // segment + 1 section

    std::vector<std::byte> buf(total_size, std::byte{0});

    auto w32 = [&](std::size_t off, std::uint32_t v) { std::memcpy(buf.data() + off, &v, 4); };
    auto w64 = [&](std::size_t off, std::uint64_t v) { std::memcpy(buf.data() + off, &v, 8); };

    // MachHeader64
    w32(0, mh_magic_64);
    w32(4, 0x0100000C);  // cputype = CPU_TYPE_ARM64 (doesn't matter for scan)
    w32(12, 2);           // filetype = MH_EXECUTE
    w32(16, 1);           // ncmds = 1
    w32(20, cmd_size);    // sizeofcmds

    // SegmentCommand64 at 0x20
    constexpr std::size_t seg_off = hdr_size;
    w32(seg_off + 0, lc_segment_64);
    w32(seg_off + 4, cmd_size);
    std::memcpy(buf.data() + seg_off + 8, "__TEXT", 6); // segname
    w64(seg_off + 24, text_vmaddr);                      // vmaddr
    w64(seg_off + 32, text_raw_size);                    // vmsize
    w64(seg_off + 40, text_raw_off);                     // fileoff
    w64(seg_off + 48, text_raw_size);                    // filesize
    w32(seg_off + 64, 1);                                // nsects

    // Section64 at 0x68
    constexpr std::size_t sect_off = seg_off + seg_size;
    std::memcpy(buf.data() + sect_off + 0, "__text", 6);  // sectname
    std::memcpy(buf.data() + sect_off + 16, "__TEXT", 6); // segname
    w64(sect_off + 32, text_vmaddr);                       // addr
    w64(sect_off + 40, text_raw_size);                     // size
    w32(sect_off + 48, static_cast<std::uint32_t>(text_raw_off)); // offset (file offset)

    // Section data
    const std::size_t copy_len = std::min(code_bytes.size(), text_raw_size);
    for (std::size_t i = 0; i < copy_len; ++i)
        buf[text_raw_off + i] = std::byte{code_bytes[i]};

    return buf;
}

// ---- tests -----------------------------------------------------------------

TEST(unit_test_macho_memory_file_scan, finds_pattern)
{
    const std::vector<std::uint8_t> code = {0x55, 0x48, 0x89, 0xE5, 0xC3};
    const auto buf = make_macho64_with_text_section(code);

    const auto result =
            MachOPatternScanner::scan_for_pattern_in_memory_file(std::span<const std::byte>{buf}, "55 48 89 E5");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->target_offset, 0);
}

TEST(unit_test_macho_memory_file_scan, finds_pattern_with_wildcard)
{
    const std::vector<std::uint8_t> code = {0xDE, 0xAD, 0xBE, 0xEF};
    const auto buf = make_macho64_with_text_section(code);

    const auto result =
            MachOPatternScanner::scan_for_pattern_in_memory_file(std::span<const std::byte>{buf}, "DE ?? BE EF");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->target_offset, 0);
}

TEST(unit_test_macho_memory_file_scan, pattern_not_found_returns_nullopt)
{
    const std::vector<std::uint8_t> code = {0x01, 0x02, 0x03};
    const auto buf = make_macho64_with_text_section(code);

    const auto result =
            MachOPatternScanner::scan_for_pattern_in_memory_file(std::span<const std::byte>{buf}, "AA BB CC");

    EXPECT_FALSE(result.has_value());
}

TEST(unit_test_macho_memory_file_scan, invalid_data_returns_nullopt)
{
    const std::vector<std::byte> garbage(64, std::byte{0xFF});
    const auto result =
            MachOPatternScanner::scan_for_pattern_in_memory_file(std::span<const std::byte>{garbage}, "FF FF");
    EXPECT_FALSE(result.has_value());
}

TEST(unit_test_macho_memory_file_scan, empty_data_returns_nullopt)
{
    const auto result = MachOPatternScanner::scan_for_pattern_in_memory_file({}, "FF");
    EXPECT_FALSE(result.has_value());
}

TEST(unit_test_macho_memory_file_scan, raw_addr_and_virtual_addr_correct)
{
    const std::vector<std::uint8_t> code = {0xCA, 0xFE, 0xBA, 0xBE};
    const auto buf = make_macho64_with_text_section(code);

    constexpr std::size_t expected_raw_off = 32 + 72 + 80; // hdr + seg + sect_hdr
    const auto result =
            MachOPatternScanner::scan_for_pattern_in_memory_file(std::span<const std::byte>{buf}, "CA FE BA BE");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->raw_base_addr, expected_raw_off);
    EXPECT_EQ(result->virtual_base_addr, 0x100001000ULL);
}
