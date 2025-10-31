# `omath::PatternScanner` — Fast byte-pattern search with wildcards

> Header: your project’s `pattern_scanner.hpp`
> Namespace: `omath`
> Core API: `scan_for_pattern(...)` (span or iterators)
> Errors: `PatternScanError::INVALID_PATTERN_STRING` (from `parse_pattern`)
> C++: uses `std::span`, `std::expected`, `std::byte`

`PatternScanner` scans a contiguous byte range for a **hex pattern** that may include **wildcards**. It returns an iterator to the **first match** or the end iterator if not found (or if the pattern string is invalid).

---

## Quick start

```cpp
#include "pattern_scanner.hpp"
using omath::PatternScanner;

std::vector<std::byte> buf = /* ... bytes ... */;
std::span<std::byte>    s{buf.data(), buf.size()};

// Example pattern: "48 8B ?? ?? 89" (hex bytes with '?' as any byte)
auto it = PatternScanner::scan_for_pattern(s, "48 8B ?? ?? 89");
if (it != s.end()) {
  // Found at offset:
  auto offset = static_cast<std::size_t>(it - s.begin());
}
```

Or with iterators:

```cpp
auto it = PatternScanner::scan_for_pattern(buf.begin(), buf.end(), "DE AD BE EF");
if (it != buf.end()) { /* ... */ }
```

---

## Pattern string grammar

The pattern string is parsed into a sequence of byte **tokens**:

* **Hex byte**: two hexadecimal digits form one byte.
  Examples: `90`, `4F`, `00`, `ff`
* **Wildcard byte**: `?` or `??` matches **any single byte**.
* **Separators**: any ASCII whitespace (space, tab, newline) is **ignored** and may be used to group tokens.

> ✔️ Valid: `"48 8B ?? 05 00"`, `"90 90 90"`, `"??"`, `"00??FF"` (no spaces)
> ❌ Invalid: odd number of hex digits in a token, non-hex characters (besides `?` and whitespace)

If the string cannot be parsed into a clean sequence of tokens, `parse_pattern()` returns `std::unexpected(PatternScanError::INVALID_PATTERN_STRING)`, and the public scan function returns **end**.

---

## API

```cpp
namespace omath {

enum class PatternScanError { INVALID_PATTERN_STRING };

class PatternScanner final {
public:
  // Contiguous range (span) overload
  [[nodiscard]]
  static std::span<std::byte>::iterator
  scan_for_pattern(const std::span<std::byte>& range,
                   const std::string_view& pattern);

  // Deleted rvalue-span overload (prevents dangling)
  static std::span<std::byte>::iterator
  scan_for_pattern(std::span<std::byte>&&,
                   const std::string_view&) = delete;

  // Iterator overload
  template<class IteratorType>
  requires std::input_or_output_iterator<std::remove_cvref_t<IteratorType>>
  static IteratorType
  scan_for_pattern(const IteratorType& begin,
                   const IteratorType& end,
                   const std::string_view& pattern);
private:
  [[nodiscard]]
  static std::expected<std::vector<std::optional<std::byte>>, PatternScanError>
  parse_pattern(const std::string_view& pattern_string);
};

} // namespace omath
```

### Return value

* On success: iterator to the **first** matching position.
* On failure / not found / invalid pattern: **`end`**.

---

## Complexity

Let `N` be the number of bytes in the range and `M` the number of **pattern tokens**.

* Time: **O(N × M)** (simple sliding window with early break).
* Space: **O(M)** for the parsed pattern vector.

---

## Examples

### Find a function prologue

```cpp
// x86-64: push rbp; mov rbp, rsp
auto it = PatternScanner::scan_for_pattern(s, "55 48 89 E5");
if (it != s.end()) {
  // ... process
}
```

### Skip variable bytes with wildcards

```cpp
// mov rax, <imm32>; call <rel32>
auto it = PatternScanner::scan_for_pattern(s, "48 B8 ?? ?? ?? ?? ?? ?? ?? ?? E8 ?? ?? ?? ??");
```

### Iterator-based scan (subrange)

```cpp
auto sub_begin = buf.begin() + 1024;
auto sub_end   = buf.begin() + 4096;
auto it = PatternScanner::scan_for_pattern(sub_begin, sub_end, "DE AD ?? BE EF");
```

---

## Behavior & edge cases

* **Empty or too-short**: If the effective pattern token count `M` is `0` or `M > N`, the function returns `end`.
* **Wildcards**: Each `?`/`??` token matches **exactly one** byte.
* **No exceptions**: Invalid pattern → parsed as `unexpected`, public API returns `end`.
* **No ownership**: The span overload takes `const std::span<std::byte>&` and returns a **mutable iterator** into that span. You must ensure the underlying memory stays alive. The rvalue-span overload is **deleted** to prevent dangling.

---

## Notes & caveats (implementation-sensitive)

* Although the iterator overload is constrained with `std::input_or_output_iterator`, the implementation uses `*(begin + i + j)` and arithmetic on iterators. In practice this means you should pass **random-access / contiguous iterators** (e.g., from `std::vector<std::byte>` or `std::span<std::byte>`). Using non-random-access iterators would be ill-formed.
* The inner matching loop compares a parsed token (`std::optional<std::byte>`) to the candidate byte; `std::nullopt` is treated as a **wildcard** (always matches).
* **Implementation note:** the outer scan currently derives its scan bound from the **pattern string length**; conceptually it should use the **number of parsed tokens** (hex/wildcard bytes). If you plan to accept spaces (or other non-byte characters) in the pattern string, ensure the scan window uses the parsed token count to avoid false negatives near the end of the buffer.

---

## Testing hooks

The class befriends several unit tests:

```
unit_test_pattern_scan_read_test_Test
unit_test_pattern_scan_corner_case_1_Test
unit_test_pattern_scan_corner_case_2_Test
unit_test_pattern_scan_corner_case_3_Test
unit_test_pattern_scan_corner_case_4_Test
```

Use these to validate parsing correctness, wildcard handling, and boundary conditions.

---

## Troubleshooting

* **Always returns end**: verify the pattern string is valid hex/wildcards and that you’re scanning the intended subrange. Try a simpler pattern (e.g., a single known byte) to sanity-check.
* **Crashes or compile errors with iterator overload**: use iterators that support random access (e.g., from `std::vector`), or prefer the `std::span` overload.
* **Ambiguous wildcards**: this scanner treats `?` and `??` as **byte-wide** wildcards (not per-nibble). If you need nibble-level masks, extend `parse_pattern` to support patterns like `A?`/`?F` with bitmask matching.

---

## Minimal unit test sketch

```cpp
TEST(pattern, basic) {
  std::array<std::byte, 8> data{
    std::byte{0x48}, std::byte{0x8B}, std::byte{0x01}, std::byte{0x02},
    std::byte{0x89}, std::byte{0x50}, std::byte{0x90}, std::byte{0xCC}
  };
  std::span<std::byte> s{data.data(), data.size()};
  auto it = omath::PatternScanner::scan_for_pattern(s, "48 8B ?? ?? 89");
  ASSERT_NE(it, s.end());
  EXPECT_EQ(static_cast<size_t>(it - s.begin()), 0U);
}
```

---

*Last updated: 31 Oct 2025*
