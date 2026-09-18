# `omath::Color` — RGBA color with HSV helpers (C++23)

> Header: `omath/utility/color.hpp`
> Namespace: `omath`
> Storage: a private `Vector4<float>` (`x=r`, `y=g`, `z=b`, `w=a`), exposed read-only via `value()`
> Depends on: `Vector4`, `<format>`, optionally ImGui (`OMATH_IMGUI_INTEGRATION`)
> Formatting: provides `std::formatter<omath::Color>` (narrow `char` only)

`Color` is a small RGBA value type built on top of `Vector4<float>`. It offers 8-bit channel construction, HSV↔RGB conversion, in-place HSV setters that keep alpha, linear blending, and string/formatter helpers.

---

## Quick start

```cpp
#include "omath/utility/color.hpp"
using omath::Color;

// RGBA in [0,1]; every channel (including alpha) is clamped to [0,1] on construction
Color c{0.2f, 0.4f, 0.8f, 0.5f};

// From 8-bit channels
auto red   = Color::from_rgba(255, 0, 0, 255);
auto green = Color::from_rgba(0, 255, 0, 160);

// From HSV (h wraps cyclically, s and v are clamped to [0,1])
auto cyan = Color::from_hsv(0.5f, 1.0f, 1.0f);   // a = 1

// Read/modify via HSV
auto hsv = cyan.to_hsv();     // hue ∈ [0,1), saturation ∈ [0,1], value ∈ [0,1]
cyan.set_value(0.6f);         // converts back to RGB, alpha is preserved

// Blend linearly (lerp)
auto mid = red.blend(green, 0.5f);

// Read channels
float r = mid.value().x;

// Printable (0–255 per channel, rounded)
std::string s = std::format("{}", mid);        // "[r:128, g:128, b:0, a:208]"
std::string f = std::format("{:rgbf}", mid);   // floats in [0,1]
std::string h = std::format("{:hsv}", mid);    // "[h:..., s:..., v:...]"
```

---

## Data model

* Holds a `Vector4<float>` as a private member; `value()` returns it by const reference.
* `x` = **red**, `y` = **green**, `z` = **blue**, `w` = **alpha**.
* Every constructor clamps **all four** channels to `[0,1]` via `Vector4::clamp(0,1)`.
* `operator==` / `operator!=` compare all four channels exactly.

---

## Construction & factories

```cpp
constexpr Color() noexcept;                                          // (0,0,0,0)
constexpr Color(float r, float g, float b, float a) noexcept;        // clamped to [0,1]
constexpr explicit Color(const Vector4<float>& value) noexcept;      // clamped to [0,1]

// From 8-bit RGBA (0–255) → normalized to [0,1]
static constexpr Color from_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept;

// From HSV
struct Hsv { float hue{}, saturation{}, value{}; };

static constexpr Color from_hsv(float hue, float saturation, float value) noexcept;
static constexpr Color from_hsv(const Hsv& hsv) noexcept;    // delegates to the above
```

**HSV details**

* `from_hsv(h, s, v)`: `h` is **normalized** and **cyclic** — `1.25` and `-0.75` both mean `0.25`. `s` and `v` are clamped to `[0,1]`. **Alpha = 1.0**.
* `to_hsv()`: returns `Hsv{h,s,v}` with `h ∈ [0,1)`, `s,v ∈ [0,1]`. A grey (zero chroma) color reports `h = 0`.

---

## Mutators

```cpp
constexpr void set_hue(float h) noexcept;         // cyclic, like from_hsv
constexpr void set_saturation(float s) noexcept;  // clamped to [0,1]
constexpr void set_value(float v) noexcept;       // clamped to [0,1]

// Linear blend: (1-ratio)*this + ratio*other, ratio clamped to [0,1]
constexpr Color blend(const Color& other, float ratio) const noexcept;
```

The `set_*` mutators convert through HSV and back but **keep the current alpha**.

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
// "[r:R, g:G, b:B, a:A]" with each channel as a 0–255 integer (rounded to nearest)
std::string   to_string()      const noexcept;
std::wstring  to_wstring()     const noexcept;   // same text, widened
std::u8string to_u8string()    const noexcept;   // same text, as UTF-8

// "[r:R, g:G, b:B, a:A]" with channels as floats in [0,1]
std::string   to_rgbf_string() const noexcept;

// "[h:H, s:S, v:V]" with normalized HSV floats
std::string   to_hsv_string()  const noexcept;
```

`std::formatter<omath::Color>` accepts three format specifiers:

| Spec       | Output                | Equivalent         |
|------------|-----------------------|--------------------|
| `{}` / `{:rgb}` | 0–255 integers   | `to_string()`      |
| `{:rgbf}`  | floats in [0,1]       | `to_rgbf_string()` |
| `{:hsv}`   | normalized HSV floats | `to_hsv_string()`  |

Any other specifier throws `std::format_error`. The formatter is specialized for `char` only; use `to_wstring()` / `to_u8string()` for other character types.

---

## ImGui (optional)

```cpp
#ifdef OMATH_IMGUI_INTEGRATION
ImColor to_im_color() const noexcept;   // constructs from Vector4::to_im_vec4()
#endif
```

---

## Notes & caveats

* **Blend space:** `blend` is a **linear** interpolation in RGBA; it is not perceptually uniform.
* **Hue range:** the API consistently uses **normalized hue**. Convert degrees ↔ normalized as `h_norm = h_deg / 360.f`.

---

## API summary

```cpp
struct Hsv { float hue{}, saturation{}, value{}; };

class Color final {
public:
  constexpr Color() noexcept;
  constexpr Color(float r, float g, float b, float a) noexcept;
  constexpr explicit Color(const Vector4<float>& value) noexcept;

  constexpr const Vector4<float>& value() const;
  constexpr bool operator==(const Color& other) const noexcept;

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

  std::string   to_string()      const noexcept;
  std::string   to_rgbf_string() const noexcept;
  std::string   to_hsv_string()  const noexcept;
  std::wstring  to_wstring()     const noexcept;
  std::u8string to_u8string()    const noexcept;
};

// std::formatter<omath::Color> provided: {}, {:rgb}, {:rgbf}, {:hsv}
```

---

*Last updated: 18 Sep 2026*
