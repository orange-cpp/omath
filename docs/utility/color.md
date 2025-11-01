# `omath::Color` — RGBA color with HSV helpers (C++20/23)

> Header: your project’s `color.hpp`
> Namespace: `omath`
> Inherits: `Vector4<float>` (`x=r`, `y=g`, `z=b`, `w=a`)
> Depends on: `<cstdint>`, `Vector4`, optionally ImGui (`OMATH_IMGUI_INTEGRATION`)
> Formatting: provides `std::formatter<omath::Color>`

`Color` is a tiny RGBA utility on top of `Vector4<float>`. It offers sRGB-style channel construction, HSV↔RGB conversion, in-place HSV setters, linear blending, and string/formatter helpers.

---

## Quick start

```cpp
#include "color.hpp"
using omath::Color;

// RGBA in [0,1] (r,g,b clamped to [0,1] on construction)
Color c{0.2f, 0.4f, 0.8f, 0.5f};

// From 8-bit channels
auto red   = Color::from_rgba(255, 0, 0, 255);
auto green = Color::from_rgba(0, 255, 0, 160);

// From HSV (h ∈ [0,1], s ∈ [0,1], v ∈ [0,1])
auto cyan = Color::from_hsv(0.5f, 1.0f, 1.0f);   // a = 1

// Read/modify via HSV
auto hsv   = cyan.to_hsv();       // hue ∈ [0,1], saturation ∈ [0,1], value ∈ [0,1]
cyan.set_value(0.6f);             // converts back to RGB (alpha becomes 1)

// Blend linearly (lerp)
auto mid = red.blend(green, 0.5f);

// Printable (0–255 per channel)
std::string s = std::format("{}", mid);   // "[r:128, g:128, b:0, a:207]" for example
```

---

## Data model

* Inherits `Vector4<float>`:

    * `x` = **red**, `y` = **green**, `z` = **blue**, `w` = **alpha**.
* Construction clamps **RGB** to `[0,1]` (via `Vector4::clamp(0,1)`), **alpha is not clamped** by that call (see notes).

---

## Construction & factories

```cpp
// RGBA in [0,1] (RGB clamped to [0,1]; alpha untouched by clamp)
constexpr Color(float r, float g, float b, float a) noexcept;

// Default
constexpr Color() noexcept;

// From 8-bit RGBA (0–255) → normalized to [0,1]
constexpr static Color from_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept;

// From HSV where hue ∈ [0,1], saturation ∈ [0,1], value ∈ [0,1]
struct Hsv { float hue{}, saturation{}, value{}; };

constexpr static Color from_hsv(float hue, float saturation, float value) noexcept;
constexpr static Color from_hsv(const Hsv& hsv) noexcept;    // delegates to the above

// Construct from a Vector4 (RGB clamped, alpha not clamped)
constexpr explicit Color(const Vector4& vec) noexcept;
```

**HSV details**

* `from_hsv(h, s, v)`: `h` is **normalized** (`[0,1]`); it is clamped, then mapped to the 6 hue sectors; **alpha = 1.0**.
* `to_hsv()`: returns `Hsv{h,s,v}` with **`h ∈ [0,1]`** (internally computes degrees and divides by 360), `s,v ∈ [0,1]`.

---

## Mutators

```cpp
constexpr void set_hue(float h) noexcept;         // h ∈ [0,1] recommended
constexpr void set_saturation(float s) noexcept;  // s ∈ [0,1]
constexpr void set_value(float v) noexcept;       // v ∈ [0,1]

// Linear blend: (1-ratio)*this + ratio*other, ratio clamped to [0,1]
constexpr Color blend(const Color& other, float ratio) const noexcept;
```

> ⚠️ **Alpha reset on HSV setters:** each `set_*` converts HSV→RGB using `from_hsv(...)`, which **sets alpha to 1.0** (overwriting previous `w`). If you need to preserve alpha:
>
> ```cpp
> float a = col.w;
> col.set_value(0.5f);
> col.w = a;
> ```

---

## Constants

```cpp
static constexpr Color red();    // (1,0,0,1)
static constexpr Color green();  // (0,1,0,1)
static constexpr Color blue();   // (0,0,1,1)
```

---

## String & formatting

```cpp
// "[r:R, g:G, b:B, a:A]" with each channel shown as 0–255 integer
std::string  to_string()  const noexcept;
std::wstring to_wstring() const noexcept;
std::u8string to_u8string() const noexcept;

// Formatter forwards to the above (char/wchar_t/char8_t)
template<> struct std::formatter<omath::Color>;
```

---

## ImGui (optional)

```cpp
#ifdef OMATH_IMGUI_INTEGRATION
ImColor to_im_color() const noexcept;   // constructs from Vector4's to_im_vec4()
#endif
```

Ensure `<imgui.h>` is included somewhere before this header when the macro is enabled.

---

## Notes & caveats

* **Alpha clamping:** `Vector4::clamp(min,max)` (called by `Color` ctors) clamps **x,y,z** only in the provided `Vector4` implementation; `w` is **left unchanged**. If you require strict `[0,1]` alpha, clamp it yourself:

  ```cpp
  col.w = std::clamp(col.w, 0.0f, 1.0f);
  ```
* **HSV range:** The API consistently uses **normalized hue** (`[0,1]`). Convert degrees ↔ normalized as `h_norm = h_deg / 360.f`.
* **Blend space:** `blend` is a **linear** interpolation in RGBA; it is not perceptually uniform.

---

## API summary

```cpp
struct Hsv { float hue{}, saturation{}, value{}; };

class Color final : public Vector4<float> {
public:
  constexpr Color(float r, float g, float b, float a) noexcept;
  constexpr Color() noexcept;
  constexpr explicit Color(const Vector4& vec) noexcept;

  static constexpr Color from_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept;
  static constexpr Color from_hsv(float hue, float saturation, float value) noexcept;
  static constexpr Color from_hsv(const Hsv& hsv) noexcept;

  constexpr Hsv   to_hsv() const noexcept;

  constexpr void  set_hue(float h) noexcept;
  constexpr void  set_saturation(float s) noexcept;
  constexpr void  set_value(float v) noexcept;

  constexpr Color blend(const Color& other, float ratio) const noexcept;

  static constexpr Color red();
  static constexpr Color green();
  static constexpr Color blue();

#ifdef OMATH_IMGUI_INTEGRATION
  ImColor to_im_color() const noexcept;
#endif

  std::string  to_string()  const noexcept;
  std::wstring to_wstring() const noexcept;
  std::u8string to_u8string() const noexcept;
};

// formatter<omath::Color> provided
```

---

*Last updated: 31 Oct 2025*
