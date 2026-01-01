// Tests for PePatternScanner::scan_for_pattern_in_loaded_module
#include <gtest/gtest.h>
#include <omath/utility/pe_pattern_scan.hpp>
#include <vector>
#include <cstdint>
#include <cstring>

using namespace omath;

static std::vector<std::uint8_t> make_fake_module(std::uint32_t base_of_code,
                                                  std::uint32_t size_code,
                                                  const std::vector<std::uint8_t>& code_bytes)
{
    // Constants
    constexpr std::uint32_t e_lfanew = 0x80;
    constexpr std::uint32_t nt_sig   = 0x4550;   // "PE\0\0"
    constexpr std::uint16_t opt_magic = 0x020B;  // PE32+
    constexpr std::uint16_t num_sections = 1;
    constexpr std::uint16_t opt_hdr_size = 0xF0; // Standard PE32+ optional header size
    constexpr std::uint32_t section_table_off = e_lfanew + 4 + 20 + opt_hdr_size; // sig(4) + FileHdr(20)
    constexpr std::uint32_t section_header_size = 40;
    constexpr std::uint32_t text_characteristics = 0x60000020; // code | execute | read

    const std::uint32_t headers_end = section_table_off + section_header_size;
    const std::uint32_t code_end = base_of_code + size_code;
    const std::uint32_t total_size = std::max(headers_end, code_end) + 0x100; // leave some padding
    std::vector<std::uint8_t> buf(total_size, 0);

    auto w16 = [&](std::size_t off, std::uint16_t v) { std::memcpy(buf.data() + off, &v, sizeof(v)); };
    auto w32 = [&](std::size_t off, std::uint32_t v) { std::memcpy(buf.data() + off, &v, sizeof(v)); };
    auto w64 = [&](std::size_t off, std::uint64_t v) { std::memcpy(buf.data() + off, &v, sizeof(v)); };

    // DOS header
    w16(0x00, 0x5A4D);          // e_magic "MZ"
    w32(0x3C, e_lfanew);        // e_lfanew

    // NT signature
    w32(e_lfanew, nt_sig);

    // FileHeader (starts at e_lfanew + 4)
    const std::size_t fh_off = e_lfanew + 4;
    w16(fh_off +  2, num_sections);    // NumberOfSections
    w16(fh_off + 16, opt_hdr_size);    // SizeOfOptionalHeader

    // OptionalHeader PE32+ (starts at e_lfanew + 4 + 20)
    const std::size_t opt_off = fh_off + 20;
    w16(opt_off + 0, opt_magic);       // Magic
    w32(opt_off + 4, size_code);       // SizeOfCode
    w32(opt_off + 16, 0);              // AddressOfEntryPoint (unused in test)
    w32(opt_off + 20, base_of_code);   // BaseOfCode
    w64(opt_off + 24, 0);              // ImageBase
    w32(opt_off + 32, 0x1000);         // SectionAlignment
    w32(opt_off + 36, 0x200);          // FileAlignment
    w32(opt_off + 56, code_end);       // SizeOfImage (simple upper bound)
    w32(opt_off + 60, headers_end);    // SizeOfHeaders
    w32(opt_off + 108, 0);             // NumberOfRvaAndSizes (0 directories)

    // Section header (.text) at section_table_off
    const std::size_t sh_off = section_table_off;
    std::memcpy(buf.data() + sh_off + 0, ".text", 5);          // Name[8]
    w32(sh_off + 8,  size_code);                               // VirtualSize
    w32(sh_off + 12, base_of_code);                            // VirtualAddress
    w32(sh_off + 16, size_code);                               // SizeOfRawData
    w32(sh_off + 20, base_of_code);                            // PointerToRawData
    w32(sh_off + 36, text_characteristics);                    // Characteristics

    // Place code bytes at BaseOfCode
    if (base_of_code + code_bytes.size() <= buf.size())
        std::memcpy(buf.data() + base_of_code, code_bytes.data(), code_bytes.size());

    return buf;
}

TEST(PePatternScanLoaded, FindsPatternAtBase)
{
    const std::vector<std::uint8_t> code = {0x90, 0x01, 0x02, 0x03, 0x04};
    auto buf = make_fake_module(0x200, static_cast<std::uint32_t>(code.size()), code);

    const auto res = PePatternScanner::scan_for_pattern_in_loaded_module(buf.data(), "90 01 02");
    ASSERT_TRUE(res.has_value());
    // address should point somewhere in our buffer; check offset
    const uintptr_t addr = res.value();
    const uintptr_t base = reinterpret_cast<uintptr_t>(buf.data());
    EXPECT_EQ(addr - base, 0x200u);
}

TEST(PePatternScanLoaded, WildcardMatches)
{
    const std::vector<std::uint8_t> code = {0xDE, 0xAD, 0xBE, 0xEF};
    constexpr std::uint32_t base_of_code = 0x300;
    auto buf = make_fake_module(base_of_code, static_cast<std::uint32_t>(code.size()), code);

    const auto res = PePatternScanner::scan_for_pattern_in_loaded_module(buf.data(), "DE ?? BE", ".text");
    ASSERT_TRUE(res.has_value());
    const uintptr_t addr = res.value();
    const uintptr_t base = reinterpret_cast<uintptr_t>(buf.data());
    EXPECT_EQ(addr - base, base_of_code);
}