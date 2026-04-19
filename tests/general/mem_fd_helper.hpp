#pragma once
// Cross-platform helper for creating binary test "files" without writing to disk where possible.
//
// Strategy:
//   - Linux (non-Android, or Android API >= 30): memfd_create → /proc/self/fd/<N>  (no disk I/O)
//   - All other platforms: anonymous temp file via std::tmpfile(), accessed via /proc/self/fd/<N>
//     on Linux, or a named temp file (cleaned up on destruction) elsewhere.
//
// Usage:
//   auto f = MemFdFile::create(myVector);
//   ASSERT_TRUE(f.valid());
//   scanner.scan_for_pattern_in_file(f.path(), ...);

#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <random>
#include <string>
#include <vector>

#if defined(__linux__)
#include <fcntl.h>
#include <unistd.h>
#if defined(__ANDROID__)
#if __ANDROID_API__ >= 30
#include <sys/mman.h>
#define OMATH_TEST_USE_MEMFD 1
#endif
// Android < 30: fall through to tmpfile() path below
#else
// Desktop Linux: memfd_create available since glibc 2.27 / kernel 3.17
#include <sys/mman.h>
#define OMATH_TEST_USE_MEMFD 1
#endif
#endif

class MemFdFile
{
public:
    MemFdFile() = default;

    ~MemFdFile()
    {
#if defined(OMATH_TEST_USE_MEMFD)
        if (m_fd >= 0)
            ::close(m_fd);
#else
        if (!m_temp_path.empty())
            std::filesystem::remove(m_temp_path);
#endif
    }

    MemFdFile(const MemFdFile&) = delete;
    MemFdFile& operator=(const MemFdFile&) = delete;

    MemFdFile(MemFdFile&& o) noexcept
        : m_path(std::move(o.m_path))
#if defined(OMATH_TEST_USE_MEMFD)
          ,
          m_fd(o.m_fd)
#else
          ,
          m_temp_path(std::move(o.m_temp_path))
#endif
    {
#if defined(OMATH_TEST_USE_MEMFD)
        o.m_fd = -1;
#else
        o.m_temp_path.clear();
#endif
    }

    [[nodiscard]] bool valid() const
    {
        return !m_path.empty();
    }

    [[nodiscard]] const std::filesystem::path& path() const
    {
        return m_path;
    }

    static MemFdFile create(const std::vector<std::uint8_t>& data)
    {
        return create(data.data(), data.size());
    }

    static MemFdFile create(const std::uint8_t* data, std::size_t size)
    {
        MemFdFile f;

#if defined(OMATH_TEST_USE_MEMFD)
        f.m_fd = static_cast<int>(::memfd_create("test_bin", 0));
        if (f.m_fd < 0)
            return f;

        if (!write_all(f.m_fd, data, size))
        {
            ::close(f.m_fd);
            f.m_fd = -1;
            return f;
        }
        f.m_path = "/proc/self/fd/" + std::to_string(f.m_fd);

#else
        // Portable fallback: write to a uniquely-named temp file and delete on destruction
        const auto tmp_dir = std::filesystem::temp_directory_path();
        std::mt19937_64 rng(std::random_device{}());
        const auto unique_name = "omath_test_" + std::to_string(rng()) + ".bin";
        f.m_temp_path = (tmp_dir / unique_name).string();
        f.m_path = f.m_temp_path;

        std::ofstream out(f.m_temp_path, std::ios::binary | std::ios::trunc);
        if (!out.is_open())
        {
            f.m_temp_path.clear();
            f.m_path.clear();
            return f;
        }
        out.write(reinterpret_cast<const char*>(data), static_cast<std::streamsize>(size));
        if (!out)
        {
            out.close();
            std::filesystem::remove(f.m_temp_path);
            f.m_temp_path.clear();
            f.m_path.clear();
        }
#endif
        return f;
    }

private:
    std::filesystem::path m_path;

#if defined(OMATH_TEST_USE_MEMFD)
    int m_fd = -1;

    static bool write_all(int fd, const std::uint8_t* data, std::size_t size)
    {
        std::size_t written = 0;
        while (written < size)
        {
            const auto n = ::write(fd, data + written, size - written);
            if (n <= 0)
                return false;
            written += static_cast<std::size_t>(n);
        }
        return true;
    }
#else
    std::string m_temp_path;
#endif
};

// ---------------------------------------------------------------------------
// Build a minimal PE binary in-memory with a single .text section.
// Layout (all offsets compile-time):
//   0x00: DOS header (64 B)   0x40: pad   0x80: NT sig   0x84: FileHeader (20 B)
//   0x98: OptionalHeader (0xF0 B)   0x188: SectionHeader (44 B)   0x1B4: section data
// ---------------------------------------------------------------------------
inline std::vector<std::uint8_t> build_minimal_pe(const std::vector<std::uint8_t>& section_bytes)
{
    constexpr std::uint32_t e_lfanew = 0x80u;
    constexpr std::uint16_t size_opt = 0xF0u;
    constexpr std::size_t nt_off = e_lfanew;
    constexpr std::size_t fh_off = nt_off + 4;
    constexpr std::size_t oh_off = fh_off + 20;
    constexpr std::size_t sh_off = oh_off + size_opt;
    constexpr std::size_t data_off = sh_off + 44;

    std::vector<std::uint8_t> buf(data_off + section_bytes.size(), 0u);

    buf[0] = 'M';
    buf[1] = 'Z';
    std::memcpy(buf.data() + 0x3Cu, &e_lfanew, 4);

    buf[nt_off] = 'P';
    buf[nt_off + 1] = 'E';

    constexpr std::uint16_t machine = 0x8664u, num_sections = 1u;
    std::memcpy(buf.data() + fh_off, &machine, 2);
    std::memcpy(buf.data() + fh_off + 2, &num_sections, 2);
    std::memcpy(buf.data() + fh_off + 16, &size_opt, 2);

    constexpr std::uint16_t magic = 0x20Bu;
    std::memcpy(buf.data() + oh_off, &magic, 2);

    constexpr char name[8] = {'.', 't', 'e', 'x', 't', 0, 0, 0};
    std::memcpy(buf.data() + sh_off, name, 8);

    const auto vsize = static_cast<std::uint32_t>(section_bytes.size());
    constexpr std::uint32_t vaddr = 0x1000u;
    constexpr auto ptr_raw = static_cast<std::uint32_t>(data_off);
    std::memcpy(buf.data() + sh_off + 8, &vsize, 4);
    std::memcpy(buf.data() + sh_off + 12, &vaddr, 4);
    std::memcpy(buf.data() + sh_off + 16, &vsize, 4);
    std::memcpy(buf.data() + sh_off + 20, &ptr_raw, 4);

    std::memcpy(buf.data() + data_off, section_bytes.data(), section_bytes.size());
    return buf;
}
