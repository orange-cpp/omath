# `omath::Vector3` — 3D vector (C++20/23)

> Header: your project’s `vector3.hpp`
> Namespace: `omath`
> Template: `template<class Type> requires std::is_arithmetic_v<Type>`
> Depends on: `omath::Vector2<Type>` (base class), `omath::Angle` (for `angle_between`)
> C++: uses `std::expected` ⇒ **C++23** recommended (or a backport)

`Vector3<Type>` extends `Vector2<Type>` with a `z` component and 3D operations: arithmetic, geometry (dot, cross, distance), normalization, angle-between with robust error signaling, hashing (for `float`) and `std::formatter` support.

---

## Quick start

```cpp
#include "vector3.hpp"
using omath::Vector3;

using Vec3f = Vector3<float>;

Vec3f a{3, 4, 0};
Vec3f b{1, 2, 2};

auto d     = a.distance_to(b);         // Euclidean distance
auto dot   = a.dot(b);                 // 3*1 + 4*2 + 0*2 = 11
auto cr    = a.cross(b);               // (8, -6, 2)
auto len   = a.length();               // hypot(x,y,z)
auto unit  = a.normalized();           // safe normalize (returns a if length==0)

if (auto ang = a.angle_between(b)) {
  float deg = ang->as_degrees();       // [0, 180], clamped
} else {
  // vectors have zero length -> no defined angle
}
```

---

## Data members

```cpp
Type x{0};  // inherited from Vector2<Type>
Type y{0};  // inherited from Vector2<Type>
Type z{0};
```

---

## Constructors

```cpp
constexpr Vector3() noexcept;
constexpr Vector3(const Type& x, const Type& y, const Type& z) noexcept;
```

---

## Equality & ordering

```cpp
constexpr bool operator==(const Vector3&) const noexcept; // component-wise
constexpr bool operator!=(const Vector3&) const noexcept;

bool operator< (const Vector3&) const noexcept; // compare by length()
bool operator> (const Vector3&) const noexcept;
bool operator<=(const Vector3&) const noexcept;
bool operator>=(const Vector3&) const noexcept;
```

> **Note:** Ordering uses **magnitude**, not lexicographic order.

---

## Arithmetic (mutating)

Component-wise with another vector:

```cpp
Vector3& operator+=(const Vector3&) noexcept;
Vector3& operator-=(const Vector3&) noexcept;
Vector3& operator*=(const Vector3&) noexcept;  // Hadamard product
Vector3& operator/=(const Vector3&) noexcept;
```

With a scalar:

```cpp
Vector3& operator*=(const Type& v) noexcept;
Vector3& operator/=(const Type& v) noexcept;
Vector3& operator+=(const Type& v) noexcept;
Vector3& operator-=(const Type& v) noexcept;
```

---

## Arithmetic (non-mutating)

```cpp
constexpr Vector3 operator-()                     const noexcept;
constexpr Vector3 operator+(const Vector3&)       const noexcept;
constexpr Vector3 operator-(const Vector3&)       const noexcept;
constexpr Vector3 operator*(const Vector3&)       const noexcept; // Hadamard
constexpr Vector3 operator/(const Vector3&)       const noexcept; // Hadamard
constexpr Vector3 operator*(const Type& scalar)   const noexcept;
constexpr Vector3 operator/(const Type& scalar)   const noexcept;
```

---

## Geometry & helpers

```cpp
// Distances & lengths
Type        distance_to(const Vector3&) const;          // sqrt of squared distance
constexpr Type distance_to_sqr(const Vector3&) const noexcept;
#ifndef _MSC_VER
constexpr Type length()     const;                       // hypot(x,y,z)
constexpr Type length_2d()  const;                       // 2D length from base
constexpr Vector3 normalized() const;                    // returns *this if length==0
#else
Type        length()     const noexcept;
Type        length_2d()  const noexcept;
Vector3     normalized() const noexcept;
#endif
constexpr Type length_sqr() const noexcept;

// Products
constexpr Type   dot(const Vector3&)   const noexcept;
constexpr Vector3 cross(const Vector3&) const noexcept;  // right-handed

// Sums & tuples
constexpr Type sum()      const noexcept;  // x + y + z
constexpr Type sum_2d()   const noexcept;  // x + y
constexpr auto as_tuple() const noexcept -> std::tuple<Type,Type,Type>;

// Utilities
Vector3& abs() noexcept; // component-wise absolute
```

---

## Angles & orthogonality

```cpp
enum class Vector3Error { IMPOSSIBLE_BETWEEN_ANGLE };

// Angle in degrees, clamped to [0,180]. Error if any vector has zero length.
std::expected<
  omath::Angle<float, 0.f, 180.f, AngleFlags::Clamped>,
  Vector3Error
> angle_between(const Vector3& other) const noexcept;

bool is_perpendicular(const Vector3& other) const noexcept; // true if angle == 90°
```

---

## Hashing & formatting

* **Hash (for `Vector3<float>`)**

  ```cpp
  template<> struct std::hash<omath::Vector3<float>> {
    std::size_t operator()(const omath::Vector3<float>&) const noexcept;
  };
  ```

  Example:

  ```cpp
  std::unordered_map<omath::Vector3<float>, int> counts;
  counts[{1.f, 2.f, 3.f}] = 7;
  ```

* **`std::formatter`** (all character types)

  ```cpp
  template<class Type>
  struct std::formatter<omath::Vector3<Type>>; // prints "[x, y, z]"
  ```

---

## Error handling

* `angle_between()` returns `std::unexpected(Vector3Error::IMPOSSIBLE_BETWEEN_ANGLE)` if either vector length is zero.
* Other operations are total for arithmetic `Type` (no throwing behavior in this class).

---

## Examples

### Cross product & perpendicular check

```cpp
omath::Vector3<float> x{1,0,0}, y{0,1,0};
auto z = x.cross(y);                // (0,0,1)
bool perp = x.is_perpendicular(y);  // true
```

### Safe normalization and angle

```cpp
omath::Vector3<float> u{0,0,0}, v{1,1,0};
auto nu = u.normalized();           // returns {0,0,0}
if (auto ang = u.angle_between(v)) {
  // won't happen: u has zero length → error
} else {
  // handle degenerate case
}
```

### Hadamard vs scalar multiply

```cpp
omath::Vector3<float> a{2,3,4}, b{5,6,7};
auto h = a * b;   // (10, 18, 28) component-wise
auto s = a * 2.f; // (4, 6, 8)    scalar
```

---

## API summary (signatures)

```cpp
// Ctors
constexpr Vector3() noexcept;
constexpr Vector3(const Type& x, const Type& y, const Type& z) noexcept;

// Equality & ordering
constexpr bool operator==(const Vector3&) const noexcept;
constexpr bool operator!=(const Vector3&) const noexcept;
bool operator< (const Vector3&) const noexcept;
bool operator> (const Vector3&) const noexcept;
bool operator<=(const Vector3&) const noexcept;
bool operator>=(const Vector3&) const noexcept;

// Mutating arithmetic
Vector3& operator+=(const Vector3&) noexcept;
Vector3& operator-=(const Vector3&) noexcept;
Vector3& operator*=(const Vector3&) noexcept;
Vector3& operator/=(const Vector3&) noexcept;
Vector3& operator*=(const Type&)   noexcept;
Vector3& operator/=(const Type&)   noexcept;
Vector3& operator+=(const Type&)   noexcept;
Vector3& operator-=(const Type&)   noexcept;

// Non-mutating arithmetic
constexpr Vector3 operator-() const noexcept;
constexpr Vector3 operator+(const Vector3&) const noexcept;
constexpr Vector3 operator-(const Vector3&) const noexcept;
constexpr Vector3 operator*(const Vector3&) const noexcept;
constexpr Vector3 operator/(const Vector3&) const noexcept;
constexpr Vector3 operator*(const Type&)   const noexcept;
constexpr Vector3 operator/(const Type&)   const noexcept;

// Geometry
Type        distance_to(const Vector3&) const;
constexpr Type distance_to_sqr(const Vector3&) const noexcept;
#ifndef _MSC_VER
constexpr Type   length() const;
constexpr Type   length_2d() const;
constexpr Vector3 normalized() const;
#else
Type        length() const noexcept;
Type        length_2d() const noexcept;
Vector3     normalized() const noexcept;
#endif
constexpr Type length_sqr() const noexcept;
constexpr Type dot(const Vector3&) const noexcept;
constexpr Vector3 cross(const Vector3&) const noexcept;

Vector3&    abs() noexcept;
constexpr Type sum()    const noexcept;
constexpr Type sum_2d() const noexcept;
constexpr auto as_tuple() const noexcept -> std::tuple<Type,Type,Type>;

// Angles
std::expected<omath::Angle<float,0.f,180.f,AngleFlags::Clamped>, omath::Vector3Error>
angle_between(const Vector3&) const noexcept;
bool is_perpendicular(const Vector3&) const noexcept;

// Hash (float) and formatter specializations provided below the class
```

---

## Notes

* Inherits all public API of `Vector2<Type>` (including `x`, `y`, many operators, and helpers used internally).
* `normalized()` returns the original vector if its length is zero (no NaNs).
* `cross()` uses the standard right-handed definition.
* `length()`/`normalized()` are `constexpr` on non-MSVC; MSVC builds provide `noexcept` runtime versions.

---

## See Also

- [Vector2 Documentation](vector2.md) - 2D vector operations
- [Vector4 Documentation](vector4.md) - 4D vector operations
- [Angle Documentation](../trigonometry/angle.md) - Working with angles
- [Getting Started Guide](../getting_started.md) - Quick start with OMath
- [Tutorials](../tutorials.md) - Practical examples including vector math

---

*Last updated: 1 Nov 2025*
