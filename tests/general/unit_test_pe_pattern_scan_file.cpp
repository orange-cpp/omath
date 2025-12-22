// Unit test for PePatternScanner::scan_for_pattern_in_file using a synthetic PE-like file
#include <gtest/gtest.h>
#include <omath/utility/pe_pattern_scan.hpp>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cstring>

using namespace omath;

// Helper: write a trivial PE-like file with DOS header and a single section named .text
static bool write_minimal_pe_file(const std::string& path, const std::vector<std::uint8_t>& section_bytes)
{
    std::ofstream f(path, std::ios::binary);
    if (!f.is_open()) return false;

    // Write DOS header (e_magic = 0x5A4D, e_lfanew at offset 0x3C)
    std::vector<std::uint8_t> dos(64, 0);
    dos[0] = 'M'; dos[1] = 'Z';
    // e_lfanew -> place NT headers right after DOS (offset 0x80)
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
    f.put('P'); f.put('E'); f.put('\0'); f.put('\0');

    // FileHeader: machine, num_sections
    std::uint16_t machine = 0x8664; // x64
    std::uint16_t num_sections = 1;
    std::uint32_t dummy32 = 0;
    std::uint32_t dummy32b = 0;
    std::uint16_t size_optional = 0xF0; // reasonable
    std::uint16_t characteristics = 0;
    f.write(reinterpret_cast<const char*>(&machine), sizeof(machine));
    f.write(reinterpret_cast<const char*>(&num_sections), sizeof(num_sections));
    f.write(reinterpret_cast<const char*>(&dummy32), sizeof(dummy32));
    f.write(reinterpret_cast<const char*>(&dummy32b), sizeof(dummy32b));
    std::uint32_t num_symbols = 0;
    f.write(reinterpret_cast<const char*>(&num_symbols), sizeof(num_symbols));
    f.write(reinterpret_cast<const char*>(&size_optional), sizeof(size_optional));
    f.write(reinterpret_cast<const char*>(&characteristics), sizeof(characteristics));

    // OptionalHeader (x64) minimal: magic 0x20b, image_base, size_of_code, size_of_headers
    std::uint16_t magic = 0x20b;
    f.write(reinterpret_cast<const char*>(&magic), sizeof(magic));
    // filler for rest of optional header up to size_optional
    std::vector<std::uint8_t> opt(size_optional - sizeof(magic), 0);
    // set size_code near end
    // we'll set image_base and size_code fields in reasonable positions for extractor
    // For simplicity, leave zeros; extractor primarily uses optional_header.image_base and size_code later,
    // but we will craft a SectionHeader that points to raw data we append below.
    f.write(reinterpret_cast<const char*>(opt.data()), opt.size());

    // Section header (name 8 bytes, then remaining 36 bytes)
    char name[8] = {'.','t','e','x','t',0,0,0};
    f.write(name, 8);

    // Write placeholder bytes for the rest of the section header and remember its start
    const std::uint32_t section_header_rest = 36u;
    const std::streampos header_rest_pos = f.tellp();
    std::vector<char> placeholder(section_header_rest, 0);
    f.write(placeholder.data(), placeholder.size());

    // Now write section raw data and remember its file offset
    const std::streampos data_pos = f.tellp();
    f.write(reinterpret_cast<const char*>(section_bytes.data()), static_cast<std::streamsize>(section_bytes.size()));

    // Patch section header fields: virtual_size, virtual_address, size_raw_data, ptr_raw_data
    const std::uint32_t virtual_size = static_cast<std::uint32_t>(section_bytes.size());
    const std::uint32_t virtual_address = 0x1000u;
    const std::uint32_t size_raw_data = static_cast<std::uint32_t>(section_bytes.size());
    const std::uint32_t ptr_raw_data = static_cast<std::uint32_t>(data_pos);

    // Seek back to the header_rest_pos and write fields in order
    f.seekp(header_rest_pos, std::ios::beg);
    f.write(reinterpret_cast<const char*>(&virtual_size), sizeof(virtual_size));
    f.write(reinterpret_cast<const char*>(&virtual_address), sizeof(virtual_address));
    f.write(reinterpret_cast<const char*>(&size_raw_data), sizeof(size_raw_data));
    f.write(reinterpret_cast<const char*>(&ptr_raw_data), sizeof(ptr_raw_data));

    // Seek back to end for consistency
    f.seekp(0, std::ios::end);

    f.close();
    return true;
}

TEST(unit_test_pe_pattern_scan_file, ScanFindsPattern)
{
    const std::string path = "./test_minimal_pe.bin";
    std::vector<std::uint8_t> bytes = {0x55, 0x8B, 0xEC, 0x90, 0x90}; // pattern at offset 0
    ASSERT_TRUE(write_minimal_pe_file(path, bytes));

    // Diagnostic dump to help debug extraction issues
    {
        std::ifstream in(path, std::ios::binary);
        ASSERT_TRUE(in.is_open());
        std::vector<uint8_t> head(1024, 0);
        in.read(reinterpret_cast<char*>(head.data()), static_cast<std::streamsize>(head.size()));
        std::cerr << "--- DUMP begin: " << path << " ---\n";
        for (size_t i = 0; i < head.size(); i += 16)
        {
            char buf[128];
            int pos = std::snprintf(buf, sizeof(buf), "%04zx: ", i);
            for (size_t j = 0; j < 16; ++j)
            {
                std::snprintf(buf + pos, sizeof(buf) - pos, "%02x ", head[i + j]);
                pos = std::strlen(buf);
            }
            std::cerr << buf << "\n";
        }
        std::cerr << "--- DUMP end ---\n";
    }

    const auto res = PePatternScanner::scan_for_pattern_in_file(path, "55 8B EC", ".text");
    EXPECT_TRUE(res.has_value());
}

TEST(unit_test_pe_pattern_scan_file, ScanMissingPattern)
{
    const std::string path = "./test_minimal_pe_2.bin";
    std::vector<std::uint8_t> bytes = {0x00, 0x01, 0x02, 0x03};
    ASSERT_TRUE(write_minimal_pe_file(path, bytes));

    const auto res = PePatternScanner::scan_for_pattern_in_file(path, "FF EE DD", ".text");
    EXPECT_FALSE(res.has_value());
}
