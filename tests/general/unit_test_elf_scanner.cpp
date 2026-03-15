//
// Created by Vladislav on 30.12.2025.
//
#include <algorithm>
#include <cstring>
#include <fstream>
#include <gtest/gtest.h>
#include <omath/utility/elf_pattern_scan.hpp>
#include <vector>

using namespace omath;

// ---- helpers ---------------------------------------------------------------

// Minimal ELF64 file with a single .text section containing known bytes.
// Layout:
//   0x000 : ELF64 file header  (64 bytes)
//   0x040 : section data       (padded to 0x20 bytes)
//   0x060 : section name table ".text\0" + "\0" (empty name for SHN_UNDEF)
//   0x080 : section header table  (3 entries × 64 bytes = 0xC0)
static std::vector<std::byte> make_elf64_with_text_section(const std::vector<std::uint8_t>& code_bytes)
{
    // Fixed layout constants
    constexpr std::size_t text_off = 0x40;
    constexpr std::size_t text_size = 0x20; // always 32 bytes (code padded with zeros)
    constexpr std::size_t shstrtab_off = text_off + text_size;
    // ".text\0" = 6 chars, prepend \0 for SHN_UNDEF → "\0.text\0"
    constexpr std::size_t shstrtab_size = 8; // "\0.text\0\0"
    constexpr std::size_t shdr_table_off = shstrtab_off + shstrtab_size;
    constexpr std::size_t shdr_size = 64; // sizeof(Elf64_Shdr)
    constexpr std::size_t num_sections = 3; // null + .text + .shstrtab
    constexpr std::size_t total_size = shdr_table_off + num_sections * shdr_size;

    std::vector<std::byte> buf(total_size, std::byte{0});

    auto w8 = [&](std::size_t off, std::uint8_t v) { buf[off] = std::byte{v}; };
    auto w16 = [&](std::size_t off, std::uint16_t v)
    { std::memcpy(buf.data() + off, &v, 2); };
    auto w32 = [&](std::size_t off, std::uint32_t v)
    { std::memcpy(buf.data() + off, &v, 4); };
    auto w64 = [&](std::size_t off, std::uint64_t v)
    { std::memcpy(buf.data() + off, &v, 8); };

    // --- ELF64 file header ---
    // e_ident
    buf[0] = std::byte{0x7F};
    buf[1] = std::byte{'E'};
    buf[2] = std::byte{'L'};
    buf[3] = std::byte{'F'};
    w8(4, 2); // ELFCLASS64
    w8(5, 1); // ELFDATA2LSB
    w8(6, 1); // EV_CURRENT
    // rest of e_ident is 0
    w16(16, 2);  // e_type = ET_EXEC
    w16(18, 62); // e_machine = EM_X86_64
    w32(20, 1);  // e_version
    w64(24, 0);  // e_entry
    w64(32, 0);  // e_phoff
    w64(40, static_cast<std::uint64_t>(shdr_table_off)); // e_shoff
    w32(48, 0);  // e_flags
    w16(52, 64); // e_ehsize
    w16(54, 56); // e_phentsize
    w16(56, 0);  // e_phnum
    w16(58, static_cast<std::uint16_t>(shdr_size)); // e_shentsize
    w16(60, static_cast<std::uint16_t>(num_sections)); // e_shnum
    w16(62, 2);  // e_shstrndx = 2 (.shstrtab is section index 2)

    // --- section data (.text) ---
    const std::size_t copy_len = std::min(code_bytes.size(), text_size);
    for (std::size_t i = 0; i < copy_len; ++i)
        buf[text_off + i] = std::byte{code_bytes[i]};

    // --- .shstrtab data: "\0.text\0\0" ---
    //  index 0 → "" (SHN_UNDEF name)
    //  index 1 → ".text"
    //  index 7 → ".shstrtab" (we cheat and use index 1 for .shstrtab too, fine for test)
    buf[shstrtab_off + 0] = std::byte{0};
    buf[shstrtab_off + 1] = std::byte{'.'};
    buf[shstrtab_off + 2] = std::byte{'t'};
    buf[shstrtab_off + 3] = std::byte{'e'};
    buf[shstrtab_off + 4] = std::byte{'x'};
    buf[shstrtab_off + 5] = std::byte{'t'};
    buf[shstrtab_off + 6] = std::byte{0};
    buf[shstrtab_off + 7] = std::byte{0};

    // --- section headers ---
    // Elf64_Shdr fields (all offsets relative to start of a section header):
    //  0  sh_name     (4)
    //  4  sh_type     (4)
    //  8  sh_flags    (8)
    //  16 sh_addr     (8)
    //  24 sh_offset   (8)
    //  32 sh_size     (8)
    //  40 sh_link     (4)
    //  44 sh_info     (4)
    //  48 sh_addralign(8)
    //  56 sh_entsize  (8)

    // Section 0: null
    // (all zeros – already zeroed)

    // Section 1: .text
    {
        const std::size_t base = shdr_table_off + 1 * shdr_size;
        w32(base + 0, 1);  // sh_name → index 1 in shstrtab → ".text"
        w32(base + 4, 1);  // sh_type = SHT_PROGBITS
        w64(base + 8, 6);  // sh_flags = SHF_ALLOC|SHF_EXECINSTR
        w64(base + 16, static_cast<std::uint64_t>(text_off)); // sh_addr (same as offset in test)
        w64(base + 24, static_cast<std::uint64_t>(text_off)); // sh_offset
        w64(base + 32, static_cast<std::uint64_t>(text_size)); // sh_size
        w64(base + 48, 16); // sh_addralign
    }

    // Section 2: .shstrtab
    {
        const std::size_t base = shdr_table_off + 2 * shdr_size;
        w32(base + 0, 0);  // sh_name → index 0 → "" (good enough for scanner)
        w32(base + 4, 3);  // sh_type = SHT_STRTAB
        w64(base + 24, static_cast<std::uint64_t>(shstrtab_off)); // sh_offset
        w64(base + 32, static_cast<std::uint64_t>(shstrtab_size)); // sh_size
    }

    return buf;
}

// ---- tests -----------------------------------------------------------------

TEST(unit_test_elf_pattern_scan_memory, finds_pattern)
{
    const std::vector<std::uint8_t> code = {0x55, 0x48, 0x89, 0xE5, 0xC3};
    const auto buf = make_elf64_with_text_section(code);
    const auto span = std::span<const std::byte>{buf};

    const auto result = ElfPatternScanner::scan_for_pattern_in_memory_file(span, "55 48 89 E5", ".text");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->target_offset, 0);
}

TEST(unit_test_elf_pattern_scan_memory, finds_pattern_with_wildcard)
{
    const std::vector<std::uint8_t> code = {0xDE, 0xAD, 0xBE, 0xEF, 0x00};
    const auto buf = make_elf64_with_text_section(code);

    const auto result =
            ElfPatternScanner::scan_for_pattern_in_memory_file(std::span<const std::byte>{buf}, "DE ?? BE EF", ".text");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->target_offset, 0);
}

TEST(unit_test_elf_pattern_scan_memory, pattern_not_found_returns_nullopt)
{
    const std::vector<std::uint8_t> code = {0x01, 0x02, 0x03, 0x04};
    const auto buf = make_elf64_with_text_section(code);

    const auto result =
            ElfPatternScanner::scan_for_pattern_in_memory_file(std::span<const std::byte>{buf}, "AA BB CC", ".text");

    EXPECT_FALSE(result.has_value());
}

TEST(unit_test_elf_pattern_scan_memory, invalid_data_returns_nullopt)
{
    const std::vector<std::byte> garbage(64, std::byte{0xFF});
    const auto result =
            ElfPatternScanner::scan_for_pattern_in_memory_file(std::span<const std::byte>{garbage}, "FF FF", ".text");

    EXPECT_FALSE(result.has_value());
}

TEST(unit_test_elf_pattern_scan_memory, empty_data_returns_nullopt)
{
    const auto result = ElfPatternScanner::scan_for_pattern_in_memory_file({}, "FF", ".text");
    EXPECT_FALSE(result.has_value());
}

TEST(unit_test_elf_pattern_scan_memory, missing_section_returns_nullopt)
{
    const std::vector<std::uint8_t> code = {0x90, 0x90};
    const auto buf = make_elf64_with_text_section(code);

    const auto result = ElfPatternScanner::scan_for_pattern_in_memory_file(std::span<const std::byte>{buf},
                                                                            "90 90", ".nonexistent");

    EXPECT_FALSE(result.has_value());
}

TEST(unit_test_elf_pattern_scan_memory, matches_file_scan)
{
    // Read test binary itself and compare memory scan vs file scan
    std::ifstream file("/proc/self/exe", std::ios::binary);
    if (!file.is_open())
        GTEST_SKIP() << "Cannot open /proc/self/exe";

    const std::string raw{std::istreambuf_iterator<char>(file), {}};
    std::vector<std::byte> data(raw.size());
    std::transform(raw.begin(), raw.end(), data.begin(),
                   [](char c) { return std::byte{static_cast<unsigned char>(c)}; });

    constexpr std::string_view pattern = "7F 45 4C 46"; // ELF magic at start of .text unlikely; use any known bytes
    const auto file_result = ElfPatternScanner::scan_for_pattern_in_file("/proc/self/exe", pattern, ".text");
    const auto mem_result =
            ElfPatternScanner::scan_for_pattern_in_memory_file(std::span<const std::byte>{data}, pattern, ".text");

    EXPECT_EQ(file_result.has_value(), mem_result.has_value());
    if (file_result && mem_result)
    {
        EXPECT_EQ(file_result->virtual_base_addr, mem_result->virtual_base_addr);
        EXPECT_EQ(file_result->raw_base_addr, mem_result->raw_base_addr);
        EXPECT_EQ(file_result->target_offset, mem_result->target_offset);
    }
}
