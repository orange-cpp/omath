# `omath::Angle` — templated angle with normalize/clamper + trig

> Header: `omath/trigonometry/angle.hpp`
> Namespace: `omath`
> Template: `Angle<Type = float, min = 0, max = 360, flags = AngleFlags::Normalized>`
> Requires: `std::is_arithmetic_v<Type>`
> Formatters: `std::formatter` for `char`, `wchar_t`, `char8_t` → `"{}deg"`

---

## Overview

`Angle` is a tiny value-type that stores an angle in **degrees** and automatically **normalizes** or **clamps** it into a compile-time range. It exposes conversions to/from radians, common trig (`sin/cos/tan/cot`), arithmetic with wrap/clamp semantics, and lightweight formatting.

Two behaviors via `AngleFlags`:

* `AngleFlags::Normalized` (default): values are wrapped into `[min, max]` using `angles::wrap_angle`.
* `AngleFlags::Clamped`: values are clamped to `[min, max]` using `std::clamp`.

---

## API

```cpp
namespace omath {

enum class AngleFlags { Normalized = 0, Clamped = 1 };

template<class Type = float, Type min = Type(0), Type max = Type(360),
         AngleFlags flags = AngleFlags::Normalized>
requires std::is_arithmetic_v<Type>
class Angle {
public:
  // Construction
  static constexpr Angle from_degrees(const Type& deg) noexcept;
  static constexpr Angle from_radians(const Type& rad) noexcept;
  constexpr Angle() noexcept;                  // 0 deg, adjusted by flags/range

  // Accessors / conversions (degrees stored internally)
  constexpr const Type& operator*() const noexcept;   // raw degrees reference
  constexpr Type        as_degrees()  const noexcept;
  constexpr Type        as_radians()  const noexcept;

  // Trig (computed from radians)
  Type sin() const noexcept;
  Type cos() const noexcept;
  Type tan() const noexcept;
  Type atan() const noexcept;  // atan(as_radians())  (rarely used)
  Type cot() const noexcept;   // cos()/sin()  (watch sin≈0)

  // Arithmetic (wraps or clamps per flags and [min,max])
  constexpr Angle& operator+=(const Angle&) noexcept;
  constexpr Angle& operator-=(const Angle&) noexcept;
  constexpr Angle  operator+(const Angle&) noexcept;
  constexpr Angle  operator-(const Angle&) noexcept;
  constexpr Angle  operator-()        const noexcept;

  // Comparison (partial ordering)
  constexpr std::partial_ordering operator<=>(const Angle&) const noexcept = default;
};

} // namespace omath
```

### Formatting

```cpp
std::format("{}", Angle<float>::from_degrees(45)); // "45deg"
```

Formatters exist for `char`, `wchar_t`, and `char8_t`.

---

## Usage examples

### Defaults (0–360, normalized)

```cpp
using Deg = omath::Angle<>;                     // float, [0,360], Normalized

auto a = Deg::from_degrees(370);                // -> 10deg
auto b = Deg::from_radians(omath::angles::pi);  // -> 180deg

a += Deg::from_degrees(355);                    // 10 + 355 -> 365 -> wraps -> 5deg

float s = a.sin();                              // sin(5°)
```

### Clamped range

```cpp
using Fov = omath::Angle<float, 1.f, 179.f, omath::AngleFlags::Clamped>;
auto fov = Fov::from_degrees(200.f);            // -> 179deg (clamped)
```

### Signed, normalized range

```cpp
using SignedDeg = omath::Angle<float, -180.f, 180.f, omath::AngleFlags::Normalized>;

auto x = SignedDeg::from_degrees(190.f);        // -> -170deg
auto y = SignedDeg::from_degrees(-200.f);       // -> 160deg
auto z = x + y;                                 // -170 + 160 = -10deg (wrapped if needed)
```

### Get/set raw degrees

```cpp
auto yaw = SignedDeg::from_degrees(-45.f);
float deg = *yaw;                                // same as yaw.as_degrees()
```

---

## Semantics & notes

* **Storage & units:** Internally stores **degrees** (`Type m_angle`). `as_radians()`/`from_radians()` use the project helpers in `omath::angles`.
* **Arithmetic honors policy:** `operator+=`/`-=` and the binary `+`/`-` apply **wrap** or **clamp** in `[min,max]`, mirroring construction behavior.
* **`atan()`**: returns `std::atan(as_radians())` (the arctangent of the *radian value*). This is mathematically unusual for an angle type and is rarely useful; prefer `tan()`/`atan2` in client code when solving geometry problems.
* **`cot()` / `tan()` singularities:** Near multiples where `sin() ≈ 0` or `cos() ≈ 0`, results blow up. Guard in your usage if inputs can approach these points.
* **Comparison:** `operator<=>` is defaulted. With normalization, distinct representatives can compare as expected (e.g., `-180` vs `180` in signed ranges are distinct endpoints).
* **No implicit numeric conversion:** There’s **no `operator Type()`**. Use `as_degrees()`/`as_radians()` (or `*angle`) explicitly—this intentional friction avoids unit mistakes.

---

## Customization patterns

* **Radians workflow:** Keep angles in degrees internally but wrap helper creators:

  ```cpp
  inline Deg degf(float d)  { return Deg::from_degrees(d);  }
  inline Deg radf(float r)  { return Deg::from_radians(r);  }
  ```
* **Compile-time policy:** Pick ranges/flags at the type level to enforce invariants (e.g., `YawDeg = Angle<float,-180,180,Normalized>`; `FovDeg = Angle<float,1,179,Clamped>`).

---

## Pitfalls & gotchas

* Ensure `min < max` at compile time for meaningful wrap/clamp behavior.
* For normalized signed ranges, decide whether your `wrap_angle(min,max)` treats endpoints half-open (e.g., `[-180,180)`) to avoid duplicate representations; the formatter will print the stored value verbatim.
* If you need **sum of many angles**, accumulating in radians then converting back can improve numeric stability at extreme values.

---

## Minimal tests

```cpp
using A = omath::Angle<>;
REQUIRE(A::from_degrees(360).as_degrees() == 0.f);
REQUIRE(A::from_degrees(-1).as_degrees()  == 359.f);

using S = omath::Angle<float,-180.f,180.f, omath::AngleFlags::Normalized>;
REQUIRE(S::from_degrees( 181).as_degrees() == -179.f);
REQUIRE(S::from_degrees(-181).as_degrees() ==  179.f);

using C = omath::Angle<float, 10.f, 20.f, omath::AngleFlags::Clamped>;
REQUIRE(C::from_degrees(5).as_degrees()    == 10.f);
REQUIRE(C::from_degrees(25).as_degrees()   == 20.f);
```

---

*Last updated: 24 Dec 2025*
