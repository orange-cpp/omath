# `omath::PePatternScanner` — Scan PE images for byte patterns

> Header: your project’s `pe_pattern_scanner.hpp`
> Namespace: `omath`
> Platform: **Windows / PE (Portable Executable) images**
> Depends on: `<filesystem>`, `<optional>`, `<cstdint>`
> Companion: works well with `omath::PatternScanner` (same pattern grammar)

`PePatternScanner` searches **Portable Executable (PE)** binaries for a hex pattern (with wildcards). You can scan:

* a **loaded module** in the current process, or
* a **PE file on disk** (by section name; defaults to **`.text`**).

---

## Pattern string grammar (same as `PatternScanner`)

* **Hex byte**: two hex digits → one byte (`90`, `4F`, `00`, `ff`).
* **Wildcard byte**: `?` or `??` matches **any byte**.
* **Whitespace**: ignored (use to group tokens).

✔️ `"48 8B ?? ?? 89"`, `"55 8B EC"`, `"??"`
❌ odd digit counts, non-hex characters (besides `?` and whitespace)

---

## API

```cpp
namespace omath {

struct PeSectionScanResult {
  std::uint64_t virtual_base_addr;  // RVA base of the scanned section (ImageBase + SectionRVA)
  std::uint64_t raw_base_addr;      // file offset (start of section in the file)
  std::ptrdiff_t target_offset;     // offset from section base to the first matched byte
};

class PePatternScanner final {
public:
  // Scan a module already loaded in *this* process.
  // module_base_address: HMODULE / ImageBase (e.g., from GetModuleHandle)
  // Returns absolute address (process VA) of the first match, or nullopt.
  static std::optional<std::uintptr_t>
  scan_for_pattern_in_loaded_module(const void* module_base_address,
                                    const std::string_view& pattern);

  // Scan a PE file on disk, by section name (default ".text").
  // Returns section bases (virtual + raw) and match offset within the section, or nullopt.
  static std::optional<PeSectionScanResult>
  scan_for_pattern_in_file(const std::filesystem::path& path_to_file,
                           const std::string_view& pattern,
                           const std::string_view& target_section_name = ".text");
};

} // namespace omath
```

---

## Return values

* **Loaded module**: `std::optional<std::uintptr_t>`

    * `value()` = **process virtual address** (ImageBase + SectionRVA + match offset).
    * `nullopt` = no match or parse/PE error.

* **File scan**: `std::optional<PeSectionScanResult>`

    * `virtual_base_addr` = **ImageBase + SectionRVA** of the scanned section (as if mapped).
    * `raw_base_addr` = **file offset** of section start.
    * `target_offset` = offset from the section base to the **first matched byte**.
    * To get addresses:

        * **Virtual (RVA)** of hit = `virtual_base_addr + target_offset`
        * **Raw file offset** of hit = `raw_base_addr + target_offset`

---

## Usage examples

### Scan a loaded module (current process)

```cpp
#include <windows.h>
#include "pe_pattern_scanner.hpp"

using omath::PePatternScanner;

auto hMod = ::GetModuleHandleW(L"kernel32.dll");
if (hMod) {
  auto addr = PePatternScanner::scan_for_pattern_in_loaded_module(
      hMod, "48 8B ?? ?? 89" // hex + wildcards
  );
  if (addr) {
    // Use the absolute process VA:
    std::uintptr_t hit_va = *addr;
    // ...
  }
}
```

### Scan a PE file on disk (default section “.text”)

```cpp
#include "pe_pattern_scanner.hpp"
using omath::PePatternScanner;

auto res = PePatternScanner::scan_for_pattern_in_file(
    R"(C:\Windows\System32\kernel32.dll)", "55 8B EC"
);
if (res) {
  auto rva_hit  = res->virtual_base_addr + res->target_offset;
  auto raw_hit  = res->raw_base_addr     + res->target_offset;
  // rva_hit: where it would be in memory; raw_hit: file offset
}
```

### Scan another section (e.g., “.rdata”)

```cpp
auto res = PePatternScanner::scan_for_pattern_in_file(
  "foo.dll", "48 8D 0D ?? ?? ?? ??", ".rdata"
);
```

---

## Notes & edge cases

* **PE only**: these functions assume a valid **PE/COFF** layout. Non-PE files or corrupted headers yield `nullopt`.
* **Section name**: `scan_for_pattern_in_file` defaults to **`.text`**; pass a different name to target other sections.
* **Alignment & RVAs**: `virtual_base_addr` is computed from section headers (RVA aligned per section alignment). The returned “virtual” base is suitable for RVA math; the **process VA** returned by the “loaded module” API already includes the image base.
* **Architecture**: works for 32-bit and 64-bit PEs; `std::uintptr_t` size matches the build architecture.
* **Performance**: Pattern matching is **O(N × M)** (sliding window with wildcards). For large images, prefer scanning only necessary sections.
* **Wildcards**: Each `?` matches **one byte** (no nibble masks). If you need `A?`-style nibble wildcards, extend the parser (see `PatternScanner`).
* **Safety**: For loaded modules, you must have access to the memory; scanning read-only sections is fine, but never write. For file scans, ensure the file path is accessible.

---

## Troubleshooting

* **`nullopt`**: Verify the pattern (valid tokens), correct **section**, and that you’re scanning the intended module/file (check bitness and version).
* **“Loaded module” address math**: If you need an **offset from the module base**, compute `offset = hit_va - reinterpret_cast<std::uintptr_t>(module_base_address)`.
* **Multiple matches**: Only the **first** match is returned. If you need all matches, extend the implementation to continue scanning from `target_offset + 1`.

---

## See also

* `omath::PatternScanner` — raw buffer/iterator scanning with the same pattern grammar.
* `omath::Triangle`, `omath::Vector3` — math types used elsewhere in the library.

---

*Last updated: 31 Oct 2025*
