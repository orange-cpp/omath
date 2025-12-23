#include <cstdint>
#include <cstring>
#include <fstream>
#include <gtest/gtest.h>
#include <omath/utility/pe_pattern_scan.hpp>
#include <vector>

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

static bool write_bytes(const std::string& path, const std::vector<std::uint8_t>& data)
{
    std::ofstream f(path, std::ios::binary);
    if (!f.is_open())
        return false;
    f.write(reinterpret_cast<const char*>(data.data()), data.size());
    return true;
}

// Helper: write a trivial PE-like file with DOS header and a single section named .text
static bool write_minimal_pe_file(const std::string& path, const std::vector<std::uint8_t>& section_bytes)
{
    std::ofstream f(path, std::ios::binary);
    if (!f.is_open())
        return false;

    // Write DOS header (e_magic = 0x5A4D, e_lfanew at offset 0x3C)
    std::vector<std::uint8_t> dos(64, 0);
    dos[0] = 'M';
    dos[1] = 'Z';
    std::uint32_t e_lfanew = 0x80;
    std::memcpy(dos.data() + 0x3C, &e_lfanew, sizeof(e_lfanew));
    f.write(reinterpret_cast<const char*>(dos.data()), dos.size());

    // Pad up to e_lfanew
    if (f.tellp() < static_cast<std::streampos>(e_lfanew))
    {
        std::vector<char> pad(e_lfanew - static_cast<std::uint32_t>(f.tellp()), 0);
        f.write(pad.data(), pad.size());
    }

    // NT headers signature 'PE\0\0'
    f.put('P');
    f.put('E');
    f.put('\0');
    f.put('\0');

    // FileHeader minimal
    std::uint16_t machine = 0x8664; // x64
    std::uint16_t num_sections = 1;
    std::uint32_t dummy32 = 0;
    std::uint32_t dummy32b = 0;
    std::uint16_t size_optional = 0xF0;
    std::uint16_t characteristics = 0;
    f.write(reinterpret_cast<const char*>(&machine), sizeof(machine));
    f.write(reinterpret_cast<const char*>(&num_sections), sizeof(num_sections));
    f.write(reinterpret_cast<const char*>(&dummy32), sizeof(dummy32));
    f.write(reinterpret_cast<const char*>(&dummy32b), sizeof(dummy32b));
    std::uint32_t num_symbols = 0;
    f.write(reinterpret_cast<const char*>(&num_symbols), sizeof(num_symbols));
    f.write(reinterpret_cast<const char*>(&size_optional), sizeof(size_optional));
    f.write(reinterpret_cast<const char*>(&characteristics), sizeof(characteristics));

    // OptionalHeader minimal filler
    std::uint16_t magic = 0x20b;
    f.write(reinterpret_cast<const char*>(&magic), sizeof(magic));
    std::vector<std::uint8_t> opt(size_optional - sizeof(magic), 0);
    f.write(reinterpret_cast<const char*>(opt.data()), opt.size());

    // Section header (name 8 bytes, then remaining 36 bytes)
    char name[8] = {'.', 't', 'e', 'x', 't', 0, 0, 0};
    f.write(name, 8);

    constexpr std::uint32_t section_header_rest = 36u;
    const std::streampos header_rest_pos = f.tellp();
    std::vector<char> placeholder(section_header_rest, 0);
    f.write(placeholder.data(), placeholder.size());

    // Now write section raw data and remember its file offset
    const std::streampos data_pos = f.tellp();
    f.write(reinterpret_cast<const char*>(section_bytes.data()), static_cast<std::streamsize>(section_bytes.size()));

    // Patch section header fields
    const std::uint32_t virtual_size = static_cast<std::uint32_t>(section_bytes.size());
    constexpr std::uint32_t virtual_address = 0x1000u;
    const std::uint32_t size_raw_data = static_cast<std::uint32_t>(section_bytes.size());
    const std::uint32_t ptr_raw_data = static_cast<std::uint32_t>(data_pos);

    f.seekp(header_rest_pos, std::ios::beg);
    f.write(reinterpret_cast<const char*>(&virtual_size), sizeof(virtual_size));
    f.write(reinterpret_cast<const char*>(&virtual_address), sizeof(virtual_address));
    f.write(reinterpret_cast<const char*>(&size_raw_data), sizeof(size_raw_data));
    f.write(reinterpret_cast<const char*>(&ptr_raw_data), sizeof(ptr_raw_data));
    f.seekp(0, std::ios::end);

    f.close();
    return true;
}

TEST(unit_test_pe_pattern_scan_more2, LoadedModuleNullBaseReturnsNull)
{
    const auto res = PePatternScanner::scan_for_pattern_in_loaded_module(nullptr, "DE AD");
    EXPECT_FALSE(res.has_value());
}

TEST(unit_test_pe_pattern_scan_more2, LoadedModuleInvalidOptionalHeaderReturnsNull)
{
    // Construct in-memory buffer with DOS header but invalid optional header magic
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

    // Place an NT header with wrong optional magic at e_lfanew
    const auto nt_ptr = buf.data() + dos->e_lfanew;
    // write signature
    nt_ptr[0] = 'P';
    nt_ptr[1] = 'E';
    nt_ptr[2] = 0;
    nt_ptr[3] = 0;
    // craft FileHeader with size_optional_header large enough
    constexpr std::uint16_t size_opt = 0xE0;
    // file header starts at offset 4
    std::memcpy(nt_ptr + 4 + 12, &size_opt,
                sizeof(size_opt)); // size_optional_header located after 12 bytes into FileHeader
    // write optional header magic to be invalid value
    constexpr std::uint16_t bad_magic = 0x9999;
    std::memcpy(nt_ptr + 4 + sizeof(std::uint32_t) + sizeof(std::uint16_t) + sizeof(std::uint16_t), &bad_magic,
                sizeof(bad_magic));

    const auto res = PePatternScanner::scan_for_pattern_in_loaded_module(buf.data(), "DE AD");
    EXPECT_FALSE(res.has_value());
}

TEST(unit_test_pe_pattern_scan_more2, FileX86OptionalHeaderScanFindsPattern)
{
    constexpr std::string path = "./test_pe_x86.bin";
    const std::vector<std::uint8_t> pattern = {0xDE, 0xAD, 0xBE, 0xEF};

    // Use helper from this file to write a consistent minimal PE file with .text section
    ASSERT_TRUE(write_minimal_pe_file(path, pattern));

    const auto res = PePatternScanner::scan_for_pattern_in_file(path, "DE AD BE EF", ".text");
    ASSERT_TRUE(res.has_value());
    EXPECT_GE(res->virtual_base_addr, 0u);
    EXPECT_GE(res->raw_base_addr, 0u);
    EXPECT_EQ(res->target_offset, 0);
}

TEST(unit_test_pe_pattern_scan_more2, FilePatternNotFoundReturnsNull)
{
    const std::string path = "./test_pe_no_pattern.bin";
    std::vector<std::uint8_t> data(512, 0);
    // minimal DOS/NT headers to make extract_section fail earlier or return empty data
    data[0] = 'M';
    data[1] = 'Z';
    constexpr std::uint32_t e_lfanew = 0x80;
    std::memcpy(data.data() + 0x3C, &e_lfanew, sizeof(e_lfanew));
    // NT signature
    data[e_lfanew + 0] = 'P';
    data[e_lfanew + 1] = 'E';
    data[e_lfanew + 2] = 0;
    data[e_lfanew + 3] = 0;
    // FileHeader: one section, size_optional_header set low
    constexpr std::uint16_t num_sections = 1;
    constexpr std::uint16_t size_optional_header = 0xE0;
    std::memcpy(data.data() + e_lfanew + 6, &num_sections, sizeof(num_sections));
    std::memcpy(data.data() + e_lfanew + 4 + 12, &size_optional_header, sizeof(size_optional_header));
    // Optional header magic x64
    constexpr std::uint16_t magic = 0x020B;
    std::memcpy(data.data() + e_lfanew + 4 + sizeof(TestFileHeader), &magic, sizeof(magic));
    // Section header .text with small data that does not contain the pattern
    constexpr std::size_t offset_to_segment_table = e_lfanew + 4 + sizeof(TestFileHeader) + size_optional_header;
    constexpr char name[8] = {'.', 't', 'e', 'x', 't', 0, 0, 0};
    std::memcpy(data.data() + offset_to_segment_table, name, 8);
    std::uint32_t vs = 4, va = 0x1000, srd = 4, prd = 0x200;
    std::memcpy(data.data() + offset_to_segment_table + 8, &vs, 4);
    std::memcpy(data.data() + offset_to_segment_table + 12, &va, 4);
    std::memcpy(data.data() + offset_to_segment_table + 16, &srd, 4);
    std::memcpy(data.data() + offset_to_segment_table + 20, &prd, 4);
    // write file
    ASSERT_TRUE(write_bytes(path, data));

    const auto res = PePatternScanner::scan_for_pattern_in_file(path, "AA BB CC", ".text");
    EXPECT_FALSE(res.has_value());
}
// Extra tests for pe_pattern_scan edge cases (on-disk API)

TEST(PePatternScanMore2, PatternAtStartFound)
{
    const std::string path = "./test_pe_more_start.bin";
    const std::vector<std::uint8_t> bytes = {0x90, 0x01, 0x02, 0x03, 0x04};
    ASSERT_TRUE(write_minimal_pe_file(path, bytes));

    const auto res = PePatternScanner::scan_for_pattern_in_file(path, "90 01 02", ".text");
    EXPECT_TRUE(res.has_value());
}

TEST(PePatternScanMore2, PatternAtEndFound)
{
    constexpr std::string path = "./test_pe_more_end.bin";
    std::vector<std::uint8_t> bytes = {0x00, 0x11, 0x22, 0x33, 0x44};
    ASSERT_TRUE(write_minimal_pe_file(path, bytes));

    const auto res = PePatternScanner::scan_for_pattern_in_file(path, "22 33 44", ".text");
    if (!res.has_value())
    {
        // Try to locate the section header and print the raw section bytes the scanner would read
        std::ifstream in(path, std::ios::binary);
        ASSERT_TRUE(in.is_open());
        // search for ".text" name
        in.seekg(0, std::ios::beg);
        std::vector<char> filebuf((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        const auto it = std::search(filebuf.begin(), filebuf.end(), std::begin(".text"), std::end(".text") - 1);
        if (it != filebuf.end())
        {
            const size_t pos = std::distance(filebuf.begin(), it);
            // after name, next fields: virtual_size (4), virtual_address(4), size_raw_data(4), ptr_raw_data(4)
            const size_t meta_off = pos + 8;
            uint32_t virtual_size{};
            uint32_t virtual_address{};
            uint32_t size_raw_data{};
            uint32_t ptr_raw_data{};
            std::memcpy(&virtual_size, filebuf.data() + meta_off, sizeof(virtual_size));
            std::memcpy(&virtual_address, filebuf.data() + meta_off + 4, sizeof(virtual_address));
            std::memcpy(&size_raw_data, filebuf.data() + meta_off + 8, sizeof(size_raw_data));
            std::memcpy(&ptr_raw_data, filebuf.data() + meta_off + 12, sizeof(ptr_raw_data));

            std::cerr << "Parsed section header: virtual_size=" << virtual_size << " virtual_address=0x" << std::hex
                      << virtual_address << std::dec << " size_raw_data=" << size_raw_data
                      << " ptr_raw_data=" << ptr_raw_data << "\n";

            if (ptr_raw_data + size_raw_data <= filebuf.size())
            {
                std::cerr << "Extracted section bytes:\n";
                for (size_t i = 0; i < size_raw_data; i += 16)
                {
                    std::fprintf(stderr, "%04zx: ", i);
                    for (size_t j = 0; j < 16 && i + j < size_raw_data; ++j)
                        std::fprintf(stderr, "%02x ", static_cast<uint8_t>(filebuf[ptr_raw_data + i + j]));
                    std::fprintf(stderr, "\n");
                }
            }
        }
    }
    EXPECT_TRUE(res.has_value());
}

TEST(PePatternScanMore2, WildcardMatches)
{
    const std::string path = "./test_pe_more_wild.bin";
    const std::vector<std::uint8_t> bytes = {0xDE, 0xAD, 0xBE, 0xEF};
    ASSERT_TRUE(write_minimal_pe_file(path, bytes));

    const auto res = PePatternScanner::scan_for_pattern_in_file(path, "DE ?? BE", ".text");
    EXPECT_TRUE(res.has_value());
}

TEST(PePatternScanMore2, PatternLongerThanBuffer)
{
    const std::string path = "./test_pe_more_small.bin";
    const std::vector<std::uint8_t> bytes = {0xAA, 0xBB};
    ASSERT_TRUE(write_minimal_pe_file(path, bytes));

    const auto res = PePatternScanner::scan_for_pattern_in_file(path, "AA BB CC", ".text");
    EXPECT_FALSE(res.has_value());
}

TEST(PePatternScanMore2, InvalidPatternParse)
{
    const std::string path = "./test_pe_more_invalid.bin";
    const std::vector<std::uint8_t> bytes = {0x01, 0x02, 0x03};
    ASSERT_TRUE(write_minimal_pe_file(path, bytes));

    const auto res = PePatternScanner::scan_for_pattern_in_file(path, "01 GG 03", ".text");
    EXPECT_FALSE(res.has_value());
}
