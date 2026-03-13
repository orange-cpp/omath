// Additional tests for PePatternScanner to exercise edge cases and loaded-module scanning
#include <cstdint>
#include <cstring>
#include <gtest/gtest.h>
#include <omath/utility/pe_pattern_scan.hpp>
#include <vector>
#include "mem_fd_helper.hpp"

using namespace omath;

TEST(unit_test_pe_pattern_scan_more, InvalidDosHeader)
{
    std::vector<std::uint8_t> data(128, 0);
    data[0] = 'N';
    data[1] = 'Z';
    const auto f = MemFdFile::create(data);
    ASSERT_TRUE(f.valid());

    const auto res = PePatternScanner::scan_for_pattern_in_file(f.path(), "55 8B EC", ".text");
    EXPECT_FALSE(res.has_value());
}

TEST(unit_test_pe_pattern_scan_more, InvalidNtSignature)
{
    std::vector<std::uint8_t> data(256, 0);
    data[0] = 'M';
    data[1] = 'Z';
    constexpr std::uint32_t e_lfanew = 0x80;
    std::memcpy(data.data() + 0x3C, &e_lfanew, sizeof(e_lfanew));
    data[e_lfanew + 0] = 'X';
    data[e_lfanew + 1] = 'Y';
    data[e_lfanew + 2] = 'Z';
    data[e_lfanew + 3] = 'W';
    const auto f = MemFdFile::create(data);
    ASSERT_TRUE(f.valid());

    const auto res = PePatternScanner::scan_for_pattern_in_file(f.path(), "55 8B EC", ".text");
    EXPECT_FALSE(res.has_value());
}

TEST(unit_test_pe_pattern_scan_more, SectionNotFound)
{
    // Minimal PE with a .data section; scanning for .text should fail
    constexpr std::uint32_t e_lfanew  = 0x80u;
    constexpr std::uint16_t size_opt  = 0xF0u;
    constexpr std::size_t   nt_off    = e_lfanew;
    constexpr std::size_t   fh_off    = nt_off + 4;
    constexpr std::size_t   oh_off    = fh_off + 20;
    constexpr std::size_t   sh_off    = oh_off + size_opt;
    constexpr std::size_t   data_off  = sh_off + 44;

    const std::vector<std::uint8_t> sec_data = {0x00, 0x01, 0x02, 0x03};
    std::vector<std::uint8_t> buf(data_off + sec_data.size(), 0u);

    buf[0] = 'M'; buf[1] = 'Z';
    std::memcpy(buf.data() + 0x3C, &e_lfanew, 4);
    buf[nt_off] = 'P'; buf[nt_off + 1] = 'E';

    const std::uint16_t machine = 0x8664u, num_sections = 1u;
    std::memcpy(buf.data() + fh_off,      &machine,       2);
    std::memcpy(buf.data() + fh_off + 2,  &num_sections,  2);
    std::memcpy(buf.data() + fh_off + 16, &size_opt,      2);

    const std::uint16_t magic = 0x20Bu;
    std::memcpy(buf.data() + oh_off, &magic, 2);

    const char name[8] = {'.','d','a','t','a',0,0,0};
    std::memcpy(buf.data() + sh_off, name, 8);

    const std::uint32_t vs = 4u, va = 0x1000u, srd = 4u, prd = static_cast<std::uint32_t>(data_off);
    std::memcpy(buf.data() + sh_off + 8,  &vs,  4);
    std::memcpy(buf.data() + sh_off + 12, &va,  4);
    std::memcpy(buf.data() + sh_off + 16, &srd, 4);
    std::memcpy(buf.data() + sh_off + 20, &prd, 4);
    std::memcpy(buf.data() + data_off, sec_data.data(), sec_data.size());

    const auto f = MemFdFile::create(buf);
    ASSERT_TRUE(f.valid());

    const auto res = PePatternScanner::scan_for_pattern_in_file(f.path(), "00 01", ".text");
    EXPECT_FALSE(res.has_value());
}

TEST(unit_test_pe_pattern_scan_more, LoadedModuleScanFinds)
{
    // Create an in-memory buffer that mimics loaded module layout
    struct DosHeader
    {
        std::uint16_t e_magic;
        std::uint16_t e_cblp;
        std::uint16_t e_cp;
        std::uint16_t e_crlc;
        std::uint16_t e_cparhdr;
        std::uint16_t e_minalloc;
        std::uint16_t e_maxalloc;
        std::uint16_t e_ss;
        std::uint16_t e_sp;
        std::uint16_t e_csum;
        std::uint16_t e_ip;
        std::uint16_t e_cs;
        std::uint16_t e_lfarlc;
        std::uint16_t e_ovno;
        std::uint16_t e_res[4];
        std::uint16_t e_oemid;
        std::uint16_t e_oeminfo;
        std::uint16_t e_res2[10];
        std::uint32_t e_lfanew;
    };
    struct FileHeader
    {
        std::uint16_t machine;
        std::uint16_t num_sections;
        std::uint32_t timedate_stamp;
        std::uint32_t ptr_symbols;
        std::uint32_t num_symbols;
        std::uint16_t size_optional_header;
        std::uint16_t characteristics;
    };
    struct OptionalHeaderX64
    {
        std::uint16_t magic;
        std::uint16_t linker_version;
        std::uint32_t size_code;
        std::uint32_t size_init_data;
        std::uint32_t size_uninit_data;
        std::uint32_t entry_point;
        std::uint32_t base_of_code;
        std::uint64_t image_base;
        std::uint32_t section_alignment;
        std::uint32_t file_alignment;
        std::uint32_t size_image;
        std::uint32_t size_headers;
        std::uint8_t pad[200];
    };
    struct SectionHeader
    {
        char name[8];
        union
        {
            std::uint32_t physical_address;
            std::uint32_t virtual_size;
        };
        std::uint32_t virtual_address;
        std::uint32_t size_raw_data;
        std::uint32_t ptr_raw_data;
        std::uint32_t ptr_relocs;
        std::uint32_t ptr_line_numbers;
        std::uint32_t num_relocs;
        std::uint32_t num_line_numbers;
        std::uint32_t characteristics;
    };
    struct ImageNtHeadersX64
    {
        std::uint32_t signature;
        FileHeader file_header;
        OptionalHeaderX64 optional_header;
    };

    const std::vector<std::uint8_t> pattern_bytes = {0xDE, 0xAD, 0xBE, 0xEF, 0x90};
    constexpr std::uint32_t base_of_code = 0x200;
    const std::uint32_t size_code = static_cast<std::uint32_t>(pattern_bytes.size());

    const std::uint32_t bufsize = 0x400 + size_code;
    std::vector<std::uint8_t> buf(bufsize, 0);

    const auto dos = reinterpret_cast<DosHeader*>(buf.data());
    dos->e_magic  = 0x5A4D;
    dos->e_lfanew = 0x80;

    const auto nt = reinterpret_cast<ImageNtHeadersX64*>(buf.data() + dos->e_lfanew);
    nt->signature                         = 0x4550;
    nt->file_header.machine               = 0x8664;
    nt->file_header.num_sections          = 1;
    nt->file_header.size_optional_header  = static_cast<std::uint16_t>(sizeof(OptionalHeaderX64));
    nt->optional_header.magic             = 0x020B;
    nt->optional_header.base_of_code      = base_of_code;
    nt->optional_header.size_code         = size_code;

    const std::size_t section_table_off =
        static_cast<std::size_t>(dos->e_lfanew) + 4 + sizeof(FileHeader) + sizeof(OptionalHeaderX64);
    nt->optional_header.size_headers = static_cast<std::uint32_t>(section_table_off + sizeof(SectionHeader));

    const auto sect = reinterpret_cast<SectionHeader*>(buf.data() + section_table_off);
    std::memset(sect, 0, sizeof(SectionHeader));
    std::memcpy(sect->name, ".text", 5);
    sect->virtual_size    = size_code;
    sect->virtual_address = base_of_code;
    sect->size_raw_data   = size_code;
    sect->ptr_raw_data    = base_of_code;
    sect->characteristics = 0x60000020;

    std::memcpy(buf.data() + base_of_code, pattern_bytes.data(), pattern_bytes.size());

    const auto res = PePatternScanner::scan_for_pattern_in_loaded_module(buf.data(), "DE AD BE EF", ".text");
    EXPECT_TRUE(res.has_value());
}
