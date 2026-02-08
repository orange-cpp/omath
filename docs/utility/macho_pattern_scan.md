# `omath::MachOPatternScanner` — Scan Mach-O binaries for byte patterns

> Header: `omath/utility/macho_pattern_scan.hpp`
> Namespace: `omath`
> Platform: **macOS / Mach-O binaries**
> Depends on: `<cstdint>`, `<filesystem>`, `<optional>`, `<string_view>`, `omath/utility/section_scan_result.hpp`
> Companion: works well with `omath::PatternScanner` (same pattern grammar)

`MachOPatternScanner` searches **Mach-O** binaries for a hex pattern (with wildcards). You can scan:

* a **loaded module** in the current process, or
* a **Mach-O file on disk** (by section name; defaults to **`__text`**).

---

## Pattern string grammar (same as `PatternScanner`)

* **Hex byte**: two hex digits → one byte (`90`, `4F`, `00`, `ff`).
* **Wildcard byte**: `?` or `??` matches **any byte**.
* **Whitespace**: ignored (use to group tokens).

✔️ `"48 8B ?? ?? 89"`, `"55 48 89 E5"`, `"??"`
❌ odd digit counts, non-hex characters (besides `?` and whitespace)

---

## API

```cpp
namespace omath {

class MachOPatternScanner final {
public:
    // Scan a module already loaded in *this* process.
    // module_base_address: base address of the loaded Mach-O image
    // Returns absolute address (process VA) of the first match, or nullopt.
    static std::optional<std::uintptr_t>
    scan_for_pattern_in_loaded_module(
        const void* module_base_address,
        const std::string_view& pattern,
        const std::string_view& target_section_name = "__text");

    // Scan a Mach-O file on disk, by section name (default "__text").
    // Returns section bases (virtual + raw) and match offset within the section, or nullopt.
    static std::optional<SectionScanResult>
    scan_for_pattern_in_file(
        const std::filesystem::path& path_to_file,
        const std::string_view& pattern,
        const std::string_view& target_section_name = "__text");
};

} // namespace omath
```

---

## Return values

* **Loaded module**: `std::optional<std::uintptr_t>`

    * `value()` = **process virtual address** of the first match.
    * `nullopt` = no match or parse/Mach-O error.

* **File scan**: `std::optional<SectionScanResult>`

    * `virtual_base_addr` = virtual address base of the scanned section.
    * `raw_base_addr` = file offset of section start.
    * `target_offset` = offset from section base to the first matched byte.
    * To get addresses:

        * **Virtual address** of hit = `virtual_base_addr + target_offset`
        * **Raw file offset** of hit = `raw_base_addr + target_offset`

---

## Usage examples

### Scan a loaded module (current process)

```cpp
#include <dlfcn.h>
#include "omath/utility/macho_pattern_scan.hpp"

using omath::MachOPatternScanner;

void* handle = dlopen("libexample.dylib", RTLD_LAZY);
if (handle) {
    auto addr = MachOPatternScanner::scan_for_pattern_in_loaded_module(
        handle, "55 48 89 E5 ?? ?? 48"
    );
    if (addr) {
        std::uintptr_t hit_va = *addr;
        // ...
    }
    dlclose(handle);
}
```

### Scan a Mach-O file on disk

```cpp
#include "omath/utility/macho_pattern_scan.hpp"
using omath::MachOPatternScanner;

auto res = MachOPatternScanner::scan_for_pattern_in_file(
    "/usr/local/lib/libexample.dylib", "55 48 89 E5"
);
if (res) {
    auto va_hit  = res->virtual_base_addr + res->target_offset;
    auto raw_hit = res->raw_base_addr     + res->target_offset;
}
```

### Scan another section (e.g., "__cstring")

```cpp
auto res = MachOPatternScanner::scan_for_pattern_in_file(
    "myapp", "48 8D 0D ?? ?? ?? ??", "__cstring"
);
```

---

## Notes & edge cases

* **Mach-O only**: these functions assume a valid Mach-O layout. Non-Mach-O files or corrupted headers yield `nullopt`.
* **Section name**: defaults to **`__text`** (note the double underscore, per Mach-O convention); pass a different name to target other sections.
* **Performance**: Pattern matching is **O(N × M)** (sliding window with wildcards). For large binaries, prefer scanning only necessary sections.
* **Architecture**: works for 64-bit Mach-O binaries (x86_64 and arm64).

---

## See also

* [`omath::PatternScanner`](pattern_scan.md) — raw buffer/iterator scanning with the same pattern grammar.
* [`omath::PePatternScanner`](pe_pattern_scan.md) — PE (Windows) binary scanner.
* [`omath::ElfPatternScanner`](elf_pattern_scan.md) — ELF (Linux) binary scanner.
* [`omath::SectionScanResult`](section_scan_result.md) — return type for file-based scans.

---

*Last updated: Feb 2026*
