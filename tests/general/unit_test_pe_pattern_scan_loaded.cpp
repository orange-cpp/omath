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
    const std::uint32_t e_lfanew = 0x80;
    const std::uint32_t total_size = e_lfanew + 0x200 + size_code + 0x100;
    std::vector<std::uint8_t> buf(total_size, 0);

    // DOS header: e_magic at 0, e_lfanew at offset 0x3C
    buf[0] = 0x4D; buf[1] = 0x5A; // 'M' 'Z' (little-endian 0x5A4D)
    std::uint32_t le = e_lfanew;
    std::memcpy(buf.data() + 0x3C, &le, sizeof(le));

    // NT signature at e_lfanew
    const std::uint32_t nt_sig = 0x4550; // 'PE\0\0'
    std::memcpy(buf.data() + e_lfanew, &nt_sig, sizeof(nt_sig));

    // FileHeader is 20 bytes: we only need to ensure its size is present; leave zeros

    // OptionalHeader magic (optional header begins at e_lfanew + 4 + sizeof(FileHeader) == e_lfanew + 24)
    const std::uint16_t opt_magic = 0x020B; // x64
    std::memcpy(buf.data() + e_lfanew + 24, &opt_magic, sizeof(opt_magic));

    // size_code is at offset 4 inside OptionalHeader -> absolute e_lfanew + 28
    std::memcpy(buf.data() + e_lfanew + 28, &size_code, sizeof(size_code));

    // base_of_code is at offset 20 inside OptionalHeader -> absolute e_lfanew + 44
    std::memcpy(buf.data() + e_lfanew + 44, &base_of_code, sizeof(base_of_code));

    // place code bytes at offset base_of_code
    if (base_of_code + code_bytes.size() <= buf.size())
        std::memcpy(buf.data() + base_of_code, code_bytes.data(), code_bytes.size());

    return buf;
}

TEST(PePatternScanLoaded, FindsPatternAtBase)
{
    std::vector<std::uint8_t> code = {0x90, 0x01, 0x02, 0x03, 0x04};
    auto buf = make_fake_module(0x200, static_cast<std::uint32_t>(code.size()), code);

    auto res = PePatternScanner::scan_for_pattern_in_loaded_module(buf.data(), "90 01 02");
    ASSERT_TRUE(res.has_value());
    // address should point somewhere in our buffer; check offset
    uintptr_t addr = res.value();
    uintptr_t base = reinterpret_cast<uintptr_t>(buf.data());
    EXPECT_EQ(addr - base, 0x200u);
}

TEST(PePatternScanLoaded, WildcardMatches)
{
    std::vector<std::uint8_t> code = {0xDE, 0xAD, 0xBE, 0xEF};
    auto buf = make_fake_module(0x300, static_cast<std::uint32_t>(code.size()), code);

    auto res = PePatternScanner::scan_for_pattern_in_loaded_module(buf.data(), "DE ?? BE");
    ASSERT_TRUE(res.has_value());
    uintptr_t addr = res.value();
    uintptr_t base = reinterpret_cast<uintptr_t>(buf.data());
    EXPECT_EQ(addr - base, 0x300u);
}
