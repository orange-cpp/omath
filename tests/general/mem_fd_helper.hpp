#pragma once
// Linux-only helper: creates an anonymous in-memory file via memfd_create.
// Usage:
//   MemFdFile f = MemFdFile::create(data.data(), data.size());
//   // use f.path() as a std::filesystem::path
//   // fd is automatically closed on destruction
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <format>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/mman.h>

class MemFdFile
{
public:
    MemFdFile() = default;

    ~MemFdFile()
    {
        if (m_fd >= 0)
            ::close(m_fd);
    }

    MemFdFile(const MemFdFile&) = delete;
    MemFdFile& operator=(const MemFdFile&) = delete;

    MemFdFile(MemFdFile&& o) noexcept : m_fd(o.m_fd), m_path(std::move(o.m_path))
    {
        o.m_fd = -1;
    }

    [[nodiscard]] bool valid() const { return m_fd >= 0; }

    [[nodiscard]] std::filesystem::path path() const { return m_path; }

    static MemFdFile create(const std::uint8_t* data, std::size_t size)
    {
        MemFdFile f;
        f.m_fd = static_cast<int>(::memfd_create("test_bin", 0));
        if (f.m_fd < 0)
            return f;

        f.m_path = std::format("/proc/self/fd/{}", f.m_fd);

        const auto* ptr = reinterpret_cast<const char*>(data);
        std::size_t written = 0;
        while (written < size)
        {
            const auto n = ::write(f.m_fd, ptr + written, size - written);
            if (n <= 0)
            {
                ::close(f.m_fd);
                f.m_fd = -1;
                return f;
            }
            written += static_cast<std::size_t>(n);
        }
        return f;
    }

    static MemFdFile create(const std::vector<std::uint8_t>& data)
    {
        return create(data.data(), data.size());
    }

private:
    int m_fd = -1;
    std::string m_path;
};

// Build a minimal PE binary in-memory with a single .text section.
// Layout (all offsets compile-time):
//   0x00: DOS header (64 B)   0x40: pad   0x80: NT sig   0x84: FileHeader (20 B)
//   0x98: OptionalHeader (0xF0 B)   0x188: SectionHeader (44 B)   0x1B4: section data
inline std::vector<std::uint8_t> build_minimal_pe(const std::vector<std::uint8_t>& section_bytes)
{
    constexpr std::uint32_t e_lfanew  = 0x80u;
    constexpr std::uint16_t size_opt  = 0xF0u;
    constexpr std::size_t   nt_off    = e_lfanew;
    constexpr std::size_t   fh_off    = nt_off + 4;
    constexpr std::size_t   oh_off    = fh_off + 20;
    constexpr std::size_t   sh_off    = oh_off + size_opt;
    constexpr std::size_t   data_off  = sh_off + 44;

    std::vector<std::uint8_t> buf(data_off + section_bytes.size(), 0u);

    buf[0] = 'M'; buf[1] = 'Z';
    std::memcpy(buf.data() + 0x3Cu, &e_lfanew, 4);

    buf[nt_off] = 'P'; buf[nt_off + 1] = 'E';

    const std::uint16_t machine = 0x8664u, num_sections = 1u;
    std::memcpy(buf.data() + fh_off,      &machine,      2);
    std::memcpy(buf.data() + fh_off + 2,  &num_sections, 2);
    std::memcpy(buf.data() + fh_off + 16, &size_opt,     2);

    const std::uint16_t magic = 0x20Bu;
    std::memcpy(buf.data() + oh_off, &magic, 2);

    const char name[8] = {'.','t','e','x','t',0,0,0};
    std::memcpy(buf.data() + sh_off, name, 8);

    const auto vsize = static_cast<std::uint32_t>(section_bytes.size());
    const std::uint32_t vaddr = 0x1000u;
    const auto ptr_raw = static_cast<std::uint32_t>(data_off);
    std::memcpy(buf.data() + sh_off + 8,  &vsize,   4);
    std::memcpy(buf.data() + sh_off + 12, &vaddr,   4);
    std::memcpy(buf.data() + sh_off + 16, &vsize,   4);
    std::memcpy(buf.data() + sh_off + 20, &ptr_raw, 4);

    std::memcpy(buf.data() + data_off, section_bytes.data(), section_bytes.size());
    return buf;
}
