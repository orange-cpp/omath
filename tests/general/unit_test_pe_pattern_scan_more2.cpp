#include <cstdint>
#include <cstring>
#include <fstream>
#include <gtest/gtest.h>
#include <omath/utility/pe_pattern_scan.hpp>
#include <vector>
#include "mem_fd_helper.hpp"

using namespace omath;

// Local minimal FileHeader used by tests when constructing raw NT headers
struct TestFileHeader
{
    std::uint16_t machine;
    std::uint16_t num_sections;
    std::uint32_t timedate_stamp;
    std::uint32_t ptr_symbols;
    std::uint32_t num_symbols;
    std::uint16_t size_optional_header;
    std::uint16_t characteristics;
};

TEST(unit_test_pe_pattern_scan_more2, LoadedModuleNullBaseReturnsNull)
{
    const auto res = PePatternScanner::scan_for_pattern_in_loaded_module(nullptr, "DE AD");
    EXPECT_FALSE(res.has_value());
}

TEST(unit_test_pe_pattern_scan_more2, LoadedModuleInvalidOptionalHeaderReturnsNull)
{
    std::vector<std::uint8_t> buf(0x200, 0);
    struct DosHeader
    {
        std::uint16_t e_magic;
        std::uint8_t pad[0x3A];
        std::uint32_t e_lfanew;
    };
    const auto dos = reinterpret_cast<DosHeader*>(buf.data());
    dos->e_magic = 0x5A4D;
    dos->e_lfanew = 0x80;

    const auto nt_ptr = buf.data() + dos->e_lfanew;
    nt_ptr[0] = 'P'; nt_ptr[1] = 'E'; nt_ptr[2] = 0; nt_ptr[3] = 0;

    constexpr std::uint16_t size_opt = 0xE0;
    std::memcpy(nt_ptr + 4 + 12, &size_opt, sizeof(size_opt));
    constexpr std::uint16_t bad_magic = 0x9999;
    std::memcpy(nt_ptr + 4 + sizeof(std::uint32_t) + sizeof(std::uint16_t) + sizeof(std::uint16_t), &bad_magic,
                sizeof(bad_magic));

    const auto res = PePatternScanner::scan_for_pattern_in_loaded_module(buf.data(), "DE AD");
    EXPECT_FALSE(res.has_value());
}

TEST(unit_test_pe_pattern_scan_more2, FileX86OptionalHeaderScanFindsPattern)
{
    const std::vector<std::uint8_t> pattern = {0xDE, 0xAD, 0xBE, 0xEF};
    const auto f = MemFdFile::create(build_minimal_pe(pattern));
    ASSERT_TRUE(f.valid());

    const auto res = PePatternScanner::scan_for_pattern_in_file(f.path(), "DE AD BE EF", ".text");
    ASSERT_TRUE(res.has_value());
    EXPECT_GE(res->virtual_base_addr, 0u);
    EXPECT_GE(res->raw_base_addr, 0u);
    EXPECT_EQ(res->target_offset, 0);
}

TEST(unit_test_pe_pattern_scan_more2, FilePatternNotFoundReturnsNull)
{
    std::vector<std::uint8_t> data(512, 0);
    data[0] = 'M'; data[1] = 'Z';
    constexpr std::uint32_t e_lfanew = 0x80;
    std::memcpy(data.data() + 0x3C, &e_lfanew, sizeof(e_lfanew));
    data[e_lfanew + 0] = 'P'; data[e_lfanew + 1] = 'E';

    constexpr std::uint16_t num_sections = 1;
    constexpr std::uint16_t size_optional_header = 0xE0;
    std::memcpy(data.data() + e_lfanew + 6, &num_sections, sizeof(num_sections));
    std::memcpy(data.data() + e_lfanew + 4 + 12, &size_optional_header, sizeof(size_optional_header));

    constexpr std::uint16_t magic = 0x020B;
    std::memcpy(data.data() + e_lfanew + 4 + sizeof(TestFileHeader), &magic, sizeof(magic));

    constexpr std::size_t offset_to_segment_table = e_lfanew + 4 + sizeof(TestFileHeader) + size_optional_header;
    constexpr char name[8] = {'.', 't', 'e', 'x', 't', 0, 0, 0};
    std::memcpy(data.data() + offset_to_segment_table, name, 8);
    std::uint32_t vs = 4, va = 0x1000, srd = 4, prd = 0x200;
    std::memcpy(data.data() + offset_to_segment_table + 8,  &vs,  4);
    std::memcpy(data.data() + offset_to_segment_table + 12, &va,  4);
    std::memcpy(data.data() + offset_to_segment_table + 16, &srd, 4);
    std::memcpy(data.data() + offset_to_segment_table + 20, &prd, 4);

    const auto f = MemFdFile::create(data);
    ASSERT_TRUE(f.valid());

    const auto res = PePatternScanner::scan_for_pattern_in_file(f.path(), "AA BB CC", ".text");
    EXPECT_FALSE(res.has_value());
}

TEST(PePatternScanMore2, PatternAtStartFound)
{
    const std::vector<std::uint8_t> bytes = {0x90, 0x01, 0x02, 0x03, 0x04};
    const auto f = MemFdFile::create(build_minimal_pe(bytes));
    ASSERT_TRUE(f.valid());

    const auto res = PePatternScanner::scan_for_pattern_in_file(f.path(), "90 01 02", ".text");
    EXPECT_TRUE(res.has_value());
}

TEST(PePatternScanMore2, PatternAtEndFound)
{
    const std::vector<std::uint8_t> bytes = {0x00, 0x11, 0x22, 0x33, 0x44};
    const auto f = MemFdFile::create(build_minimal_pe(bytes));
    ASSERT_TRUE(f.valid());

    const auto res = PePatternScanner::scan_for_pattern_in_file(f.path(), "22 33 44", ".text");
    if (!res.has_value())
    {
        // Debug: inspect section header via the memfd path
        std::ifstream in(f.path(), std::ios::binary);
        if (in.is_open())
        {
            std::vector<char> filebuf((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
            const auto it = std::search(filebuf.begin(), filebuf.end(), std::begin(".text"), std::end(".text") - 1);
            if (it != filebuf.end())
            {
                const std::size_t pos = std::distance(filebuf.begin(), it);
                const std::size_t meta_off = pos + 8;
                std::uint32_t virtual_size{}, virtual_address{}, size_raw_data{}, ptr_raw_data{};
                std::memcpy(&virtual_size,  filebuf.data() + meta_off,      sizeof(virtual_size));
                std::memcpy(&virtual_address, filebuf.data() + meta_off + 4, sizeof(virtual_address));
                std::memcpy(&size_raw_data, filebuf.data() + meta_off + 8,  sizeof(size_raw_data));
                std::memcpy(&ptr_raw_data,  filebuf.data() + meta_off + 12, sizeof(ptr_raw_data));
                std::cerr << "Parsed section header: virtual_size=" << virtual_size << " virtual_address=0x"
                          << std::hex << virtual_address << std::dec << " size_raw_data=" << size_raw_data
                          << " ptr_raw_data=" << ptr_raw_data << "\n";
            }
        }
    }
    EXPECT_TRUE(res.has_value());
}

TEST(PePatternScanMore2, WildcardMatches)
{
    const std::vector<std::uint8_t> bytes = {0xDE, 0xAD, 0xBE, 0xEF};
    const auto f = MemFdFile::create(build_minimal_pe(bytes));
    ASSERT_TRUE(f.valid());

    const auto res = PePatternScanner::scan_for_pattern_in_file(f.path(), "DE ?? BE", ".text");
    EXPECT_TRUE(res.has_value());
}

TEST(PePatternScanMore2, PatternLongerThanBuffer)
{
    const std::vector<std::uint8_t> bytes = {0xAA, 0xBB};
    const auto f = MemFdFile::create(build_minimal_pe(bytes));
    ASSERT_TRUE(f.valid());

    const auto res = PePatternScanner::scan_for_pattern_in_file(f.path(), "AA BB CC", ".text");
    EXPECT_FALSE(res.has_value());
}

TEST(PePatternScanMore2, InvalidPatternParse)
{
    const std::vector<std::uint8_t> bytes = {0x01, 0x02, 0x03};
    const auto f = MemFdFile::create(build_minimal_pe(bytes));
    ASSERT_TRUE(f.valid());

    const auto res = PePatternScanner::scan_for_pattern_in_file(f.path(), "01 GG 03", ".text");
    EXPECT_FALSE(res.has_value());
}
