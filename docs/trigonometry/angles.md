# `omath::angles` — angle conversions, FOV helpers, and wrapping

> Header: `omath/trigonometry/angles.hpp`
> Namespace: `omath::angles`
> All functions are `[[nodiscard]]` and `noexcept` where applicable.

A small set of constexpr-friendly utilities for converting between degrees/radians, converting horizontal/vertical field of view, and wrapping angles into a closed interval.

---

## API

```cpp
// Degrees ↔ Radians (Type must be floating-point)
template<class Type>
requires std::is_floating_point_v<Type>
constexpr Type radians_to_degrees(const Type& radians) noexcept;

template<class Type>
requires std::is_floating_point_v<Type>
constexpr Type degrees_to_radians(const Type& degrees) noexcept;

// FOV conversion (inputs/outputs in degrees, aspect = width/height)
template<class Type>
requires std::is_floating_point_v<Type>
Type horizontal_fov_to_vertical(const Type& horizontal_fov, const Type& aspect) noexcept;

template<class Type>
requires std::is_floating_point_v<Type>
Type vertical_fov_to_horizontal(const Type& vertical_fov, const Type& aspect) noexcept;

// Wrap angle into [min, max] (any arithmetic type)
template<class Type>
requires std::is_arithmetic_v<Type>
Type wrap_angle(const Type& angle, const Type& min, const Type& max) noexcept;
```

---

## Usage

### Degrees ↔ Radians

```cpp
float rad = omath::angles::degrees_to_radians(180.0f); // π
double deg = omath::angles::radians_to_degrees(std::numbers::pi); // 180
```

### Horizontal ↔ Vertical FOV

* `aspect` = **width / height**.
* Inputs/outputs are **degrees**.

```cpp
float hdeg = 90.0f;
float aspect = 16.0f / 9.0f;

float vdeg = omath::angles::horizontal_fov_to_vertical(hdeg, aspect); // ~58.0°
float hdeg2 = omath::angles::vertical_fov_to_horizontal(vdeg, aspect); // ≈ 90.0°
```

Formulas (in radians):

* `v = 2 * atan( tan(h/2) / aspect )`
* `h = 2 * atan( tan(v/2) * aspect )`

### Wrapping angles (or any periodic value)

Wrap any numeric `angle` into `[min, max]`:

```cpp
// Wrap degrees into [0, 360]
float a = omath::angles::wrap_angle(   370.0f, 0.0f, 360.0f); // 10
float b = omath::angles::wrap_angle(   -15.0f, 0.0f, 360.0f); // 345
// Signed range [-180,180]
float c = omath::angles::wrap_angle(  200.0f, -180.0f, 180.0f); // -160
```

---

## Notes & edge cases

* **Type requirements**

    * Converters & FOV helpers require **floating-point** `Type`.
    * `wrap_angle` accepts any arithmetic `Type` (floats or integers).
* **Aspect ratio** must be **positive** and finite. For `aspect == 0` the FOV helpers are undefined.
* **Units**: FOV functions accept/return **degrees** but compute internally in radians.
* **Wrapping interval**: Behavior assumes `max > min`. The result lies in the **closed interval** `[min, max]` with modulo arithmetic; if you need half-open behavior (e.g., `[min,max)`), adjust your range or post-process endpoint cases.
* **constexpr**: Converters are `constexpr`; FOV helpers are runtime constexpr-compatible except for `std::atan/std::tan` constraints on some standard libraries.

---

## Quick tests

```cpp
using namespace omath::angles;

static_assert(degrees_to_radians(180.0) == std::numbers::pi);
static_assert(radians_to_degrees(std::numbers::pi_v<float>) == 180.0f);

float v = horizontal_fov_to_vertical(90.0f, 16.0f/9.0f);
float h = vertical_fov_to_horizontal(v, 16.0f/9.0f);
assert(std::abs(h - 90.0f) < 1e-5f);

assert(wrap_angle(360.0f, 0.0f, 360.0f) == 0.0f || wrap_angle(360.0f, 0.0f, 360.0f) == 360.0f);
```
