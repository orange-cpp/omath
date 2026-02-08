# `omath::SectionScanResult` — File-based pattern scan result

> Header: `omath/utility/section_scan_result.hpp`
> Namespace: `omath`
> Depends on: `<cstddef>`, `<cstdint>`

`SectionScanResult` is the return type for file-based pattern scans across all binary formats (PE, ELF, Mach-O). It carries the section's virtual and raw base addresses together with the offset to the matched pattern.

---

## API

```cpp
namespace omath {

struct SectionScanResult final {
    std::uintptr_t virtual_base_addr;  // virtual address base of the scanned section
    std::uintptr_t raw_base_addr;      // file offset of the section start
    std::ptrdiff_t target_offset;      // offset from section base to the first matched byte
};

} // namespace omath
```

---

## Computing addresses from a result

```cpp
omath::SectionScanResult res = /* ... */;

// Virtual address of the match (as if the binary were loaded at its preferred base)
auto va_hit  = res.virtual_base_addr + res.target_offset;

// Raw file offset of the match
auto raw_hit = res.raw_base_addr + res.target_offset;
```

---

## Notes

* `virtual_base_addr` is computed from the section header (RVA for PE, `sh_addr` for ELF, `addr` for Mach-O).
* `raw_base_addr` is the file offset where the section data begins on disk.
* `target_offset` is always relative to the section base — add it to either address to locate the match.

---

## See also

* [`omath::PePatternScanner`](pe_pattern_scan.md) — PE (Windows) binary scanner.
* [`omath::ElfPatternScanner`](elf_pattern_scan.md) — ELF (Linux) binary scanner.
* [`omath::MachOPatternScanner`](macho_pattern_scan.md) — Mach-O (macOS) binary scanner.
* [`omath::PatternScanner`](pattern_scan.md) — raw buffer/iterator scanning.

---

*Last updated: Feb 2026*
