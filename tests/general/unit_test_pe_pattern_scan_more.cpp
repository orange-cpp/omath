// Additional tests for PePatternScanner to exercise edge cases and loaded-module scanning
#include <cstdint>
#include <cstring>
#include <fstream>
#include <gtest/gtest.h>
#include <omath/utility/pe_pattern_scan.hpp>
#include <vector>

using namespace omath;

static bool write_bytes(const std::string& path, const std::vector<std::uint8_t>& data)
{
    std::ofstream f(path, std::ios::binary);
    if (!f.is_open())
        return false;
    f.write(reinterpret_cast<const char*>(data.data()), data.size());
    return true;
}

TEST(unit_test_pe_pattern_scan_more, InvalidDosHeader)
{
    const std::string path = "./test_bad_dos.bin";
    std::vector<std::uint8_t> data(128, 0);
    // write wrong magic
    data[0] = 'N';
    data[1] = 'Z';
    ASSERT_TRUE(write_bytes(path.data(), data));

    const auto res = PePatternScanner::scan_for_pattern_in_file(path, "55 8B EC", ".text");
    EXPECT_FALSE(res.has_value());
}

TEST(unit_test_pe_pattern_scan_more, InvalidNtSignature)
{
    const std::string path = "./test_bad_nt.bin";
    std::vector<std::uint8_t> data(256, 0);
    // valid DOS header
    data[0] = 'M';
    data[1] = 'Z';
    // point e_lfanew to 0x80
    constexpr std::uint32_t e_lfanew = 0x80;
    std::memcpy(data.data() + 0x3C, &e_lfanew, sizeof(e_lfanew));
    // write garbage at e_lfanew (not 'PE\0\0')
    data[e_lfanew + 0] = 'X';
    data[e_lfanew + 1] = 'Y';
    data[e_lfanew + 2] = 'Z';
    data[e_lfanew + 3] = 'W';
    ASSERT_TRUE(write_bytes(path.data(), data));

    const auto res = PePatternScanner::scan_for_pattern_in_file(path, "55 8B EC", ".text");
    EXPECT_FALSE(res.has_value());
}

TEST(unit_test_pe_pattern_scan_more, SectionNotFound)
{
    // reuse minimal writer but with section named .data and search .text
    const std::string path = "./test_section_not_found.bin";
    std::ofstream f(path.data(), std::ios::binary);
    ASSERT_TRUE(f.is_open());
    // DOS
    std::vector<std::uint8_t> dos(64, 0);
    dos[0] = 'M';
    dos[1] = 'Z';
    std::uint32_t e_lfanew = 0x80;
    std::memcpy(dos.data() + 0x3C, &e_lfanew, sizeof(e_lfanew));
    f.write(reinterpret_cast<char*>(dos.data()), dos.size());
    // pad
    std::vector<char> pad(e_lfanew - static_cast<std::uint32_t>(f.tellp()), 0);
    f.write(pad.data(), pad.size());
    // NT sig
    f.put('P');
    f.put('E');
    f.put('\0');
    f.put('\0');
    // FileHeader minimal
    std::uint16_t machine = 0x8664;
    std::uint16_t num_sections = 1;
    std::uint32_t z = 0;
    std::uint32_t z2 = 0;
    std::uint32_t numsym = 0;
    std::uint16_t size_opt = 0xF0;
    std::uint16_t ch = 0;
    f.write(reinterpret_cast<char*>(&machine), sizeof(machine));
    f.write(reinterpret_cast<char*>(&num_sections), sizeof(num_sections));
    f.write(reinterpret_cast<char*>(&z), sizeof(z));
    f.write(reinterpret_cast<char*>(&z2), sizeof(z2));
    f.write(reinterpret_cast<char*>(&numsym), sizeof(numsym));
    f.write(reinterpret_cast<char*>(&size_opt), sizeof(size_opt));
    f.write(reinterpret_cast<char*>(&ch), sizeof(ch));
    // Optional header magic
    std::uint16_t magic = 0x20b;
    f.write(reinterpret_cast<char*>(&magic), sizeof(magic));
    std::vector<std::uint8_t> opt(size_opt - sizeof(magic), 0);
    f.write(reinterpret_cast<char*>(opt.data()), opt.size());
    // Section header named .data
    char name[8] = {'.', 'd', 'a', 't', 'a', 0, 0, 0};
    f.write(name, 8);
    std::uint32_t vs = 4, va = 0x1000, srd = 4, prd = 0x200;
    f.write(reinterpret_cast<char*>(&vs), 4);
    f.write(reinterpret_cast<char*>(&va), 4);
    f.write(reinterpret_cast<char*>(&srd), 4);
    f.write(reinterpret_cast<char*>(&prd), 4);
    std::vector<char> rest(16, 0);
    f.write(rest.data(), rest.size());
    // section bytes
    std::vector<std::uint8_t> sec = {0x00, 0x01, 0x02, 0x03};
    f.write(reinterpret_cast<char*>(sec.data()), sec.size());
    f.close();

    auto res = PePatternScanner::scan_for_pattern_in_file(path, "00 01", ".text");
    EXPECT_FALSE(res.has_value());
}

TEST(unit_test_pe_pattern_scan_more, LoadedModuleScanFinds)
{
    // Create an in-memory buffer that mimics loaded module layout
    // Define local header structs matching those in source
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
        std::uint32_t file_alignment; /* rest omitted */
        std::uint32_t size_image;
        std::uint32_t size_headers; /* keep space */
        std::uint8_t pad[200];
    };
    struct ImageNtHeadersX64
    {
        std::uint32_t signature;
        FileHeader file_header;
        OptionalHeaderX64 optional_header;
    };

    const std::vector<std::uint8_t> pattern_bytes = {0xDE, 0xAD, 0xBE, 0xEF, 0x90};
    constexpr std::uint32_t base_of_code = 0x200; // will place bytes at offset 0x200
    const std::uint32_t size_code = static_cast<std::uint32_t>(pattern_bytes.size());

    const std::uint32_t bufsize = 0x400 + size_code;
    std::vector<std::uint8_t> buf(bufsize, 0);
    // DOS header
    const auto dos = reinterpret_cast<DosHeader*>(buf.data());
    dos->e_magic = 0x5A4D;
    dos->e_lfanew = 0x80;
    // NT headers
    const auto nt = reinterpret_cast<ImageNtHeadersX64*>(buf.data() + dos->e_lfanew);
    nt->signature = 0x4550; // 'PE\0\0'
    nt->file_header.machine = 0x8664;
    nt->file_header.num_sections = 1;
    nt->optional_header.magic = 0x020B; // x64
    nt->optional_header.base_of_code = base_of_code;
    nt->optional_header.size_code = size_code;

    // place code at base_of_code
    std::memcpy(buf.data() + base_of_code, pattern_bytes.data(), pattern_bytes.size());

    const auto res = PePatternScanner::scan_for_pattern_in_loaded_module(buf.data(), "DE AD BE EF");
    EXPECT_TRUE(res.has_value());
}
