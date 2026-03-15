// Tests for PePatternScanner::scan_for_pattern_in_memory_file
#include <cstring>
#include <gtest/gtest.h>
#include <omath/utility/pe_pattern_scan.hpp>
#include <span>
#include <vector>

using namespace omath;

// Reuse the fake-module builder from unit_test_pe_pattern_scan_loaded.cpp but
// lay out the buffer as a raw PE *file* (ptr_raw_data != virtual_address).
static std::vector<std::byte> make_fake_pe_file(std::uint32_t virtual_address, std::uint32_t ptr_raw_data,
                                                std::uint32_t section_size,
                                                const std::vector<std::uint8_t>& code_bytes)
{
    constexpr std::uint32_t e_lfanew = 0x80;
    constexpr std::uint32_t nt_sig = 0x4550;
    constexpr std::uint16_t opt_magic = 0x020B; // PE32+
    constexpr std::uint16_t num_sections = 1;
    constexpr std::uint16_t opt_hdr_size = 0xF0;
    constexpr std::uint32_t section_table_off = e_lfanew + 4 + 20 + opt_hdr_size;
    constexpr std::uint32_t section_header_size = 40;

    const std::uint32_t total_size = ptr_raw_data + section_size + 0x100;
    std::vector<std::byte> buf(total_size, std::byte{0});

    auto w16 = [&](std::size_t off, std::uint16_t v) { std::memcpy(buf.data() + off, &v, 2); };
    auto w32 = [&](std::size_t off, std::uint32_t v) { std::memcpy(buf.data() + off, &v, 4); };
    auto w64 = [&](std::size_t off, std::uint64_t v) { std::memcpy(buf.data() + off, &v, 8); };

    // DOS header
    w16(0x00, 0x5A4D);
    w32(0x3C, e_lfanew);

    // NT signature
    w32(e_lfanew, nt_sig);

    // FileHeader
    const std::size_t fh_off = e_lfanew + 4;
    w16(fh_off + 2, num_sections);
    w16(fh_off + 16, opt_hdr_size);

    // OptionalHeader PE32+
    const std::size_t opt_off = fh_off + 20;
    w16(opt_off + 0, opt_magic);
    w64(opt_off + 24, 0ULL); // ImageBase = 0 to keep virtual_base_addr in 32-bit range

    // Section header (.text)
    const std::size_t sh_off = section_table_off;
    std::memcpy(buf.data() + sh_off, ".text", 5);
    w32(sh_off + 8, section_size);        // VirtualSize
    w32(sh_off + 12, virtual_address);    // VirtualAddress
    w32(sh_off + 16, section_size);       // SizeOfRawData
    w32(sh_off + 20, ptr_raw_data);       // PointerToRawData

    // Place code at raw file offset
    const std::size_t copy_len = std::min(code_bytes.size(), static_cast<std::size_t>(section_size));
    for (std::size_t i = 0; i < copy_len; ++i)
        buf[ptr_raw_data + i] = std::byte{code_bytes[i]};

    return buf;
}

// ---- tests -----------------------------------------------------------------

TEST(unit_test_pe_memory_file_scan, finds_pattern)
{
    const std::vector<std::uint8_t> code = {0x90, 0x01, 0x02, 0x03, 0x04};
    const auto buf = make_fake_pe_file(0x1000, 0x400, static_cast<std::uint32_t>(code.size()), code);

    const auto result = PePatternScanner::scan_for_pattern_in_memory_file(std::span<const std::byte>{buf}, "90 01 02");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->target_offset, 0);
    EXPECT_EQ(result->raw_base_addr, 0x400u);
}

TEST(unit_test_pe_memory_file_scan, finds_pattern_with_wildcard)
{
    const std::vector<std::uint8_t> code = {0xDE, 0xAD, 0xBE, 0xEF};
    const auto buf = make_fake_pe_file(0x2000, 0x600, static_cast<std::uint32_t>(code.size()), code);

    const auto result =
            PePatternScanner::scan_for_pattern_in_memory_file(std::span<const std::byte>{buf}, "DE ?? BE EF");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->target_offset, 0);
}

TEST(unit_test_pe_memory_file_scan, pattern_not_found_returns_nullopt)
{
    const std::vector<std::uint8_t> code = {0x01, 0x02, 0x03};
    const auto buf = make_fake_pe_file(0x1000, 0x400, static_cast<std::uint32_t>(code.size()), code);

    const auto result =
            PePatternScanner::scan_for_pattern_in_memory_file(std::span<const std::byte>{buf}, "AA BB CC");

    EXPECT_FALSE(result.has_value());
}

TEST(unit_test_pe_memory_file_scan, invalid_data_returns_nullopt)
{
    const std::vector<std::byte> garbage(128, std::byte{0xFF});
    const auto result = PePatternScanner::scan_for_pattern_in_memory_file(std::span<const std::byte>{garbage}, "FF FF");
    EXPECT_FALSE(result.has_value());
}

TEST(unit_test_pe_memory_file_scan, empty_data_returns_nullopt)
{
    const auto result = PePatternScanner::scan_for_pattern_in_memory_file({}, "FF");
    EXPECT_FALSE(result.has_value());
}

TEST(unit_test_pe_memory_file_scan, raw_addr_differs_from_virtual_address)
{
    // ptr_raw_data = 0x600, virtual_address = 0x3000 — different intentionally
    const std::vector<std::uint8_t> code = {0xCA, 0xFE, 0xBA, 0xBE};
    const auto buf = make_fake_pe_file(0x3000, 0x600, static_cast<std::uint32_t>(code.size()), code);

    const auto result =
            PePatternScanner::scan_for_pattern_in_memory_file(std::span<const std::byte>{buf}, "CA FE BA BE");

    ASSERT_TRUE(result.has_value());
    // raw_base_addr should be ptr_raw_data, not virtual_address
    EXPECT_EQ(result->raw_base_addr, 0x600u);
    // virtual_base_addr = virtual_address + image_base (image_base = 0)
    EXPECT_EQ(result->virtual_base_addr, 0x3000u);
}
