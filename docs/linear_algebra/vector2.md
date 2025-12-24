# `omath::Vector2` — 2D vector (C++20/23)

> Header: your project’s `vector2.hpp`
> Namespace: `omath`
> Template: `template<class Type> requires std::is_arithmetic_v<Type>`

`Vector2<Type>` is a lightweight, POD-like 2D math type with arithmetic, geometry helpers, comparisons, hashing (for `float`), optional ImGui interop, and `std::formatter` support.

---

## Quick start

```cpp
#include "vector2.hpp"
using omath::Vector2;

using Vec2f = Vector2<float>;

Vec2f a{3.f, 4.f};
Vec2f b{1.f, 2.f};

auto d      = a.distance_to(b);     //  ≈ 3.1623
auto dot    = a.dot(b);             //  11
auto len    = a.length();           //  5
auto unit_a = a.normalized();       //  (0.6, 0.8)

// Component-wise mutate
Vec2f c{2, 3};
c *= b;                              // c -> (2*1, 3*2) = (2, 6)

// Scalar ops (non-mutating + mutating)
auto scaled = a * 0.5f;             // (1.5, 2)
a *= 2.f;                           // (6, 8)

// Ordering by length()
bool shorter = (b < a);

// Formatted printing
std::string s = std::format("a = {}", a);  // "a = [6, 8]"
```

---

## Members

```cpp
Type x{0};
Type y{0};
```

---

## Constructors

```cpp
constexpr Vector2();                             // (0,0)
constexpr Vector2(const Type& x, const Type& y) noexcept;
```

---

## Equality & ordering

```cpp
constexpr bool operator==(const Vector2&) const noexcept; // component-wise equality
constexpr bool operator!=(const Vector2&) const noexcept;

bool operator< (const Vector2&) const noexcept; // compares by length()
bool operator> (const Vector2&) const noexcept;
bool operator<=(const Vector2&) const noexcept;
bool operator>=(const Vector2&) const noexcept;
```

> **Note:** `<`, `>`, `<=`, `>=` order vectors by **magnitude** (not lexicographically).

---

## Arithmetic

### With another vector (component-wise, **mutating**)

```cpp
Vector2& operator+=(const Vector2&) noexcept;
Vector2& operator-=(const Vector2&) noexcept;
Vector2& operator*=(const Vector2&) noexcept;  // Hadamard product (x*=x, y*=y)
Vector2& operator/=(const Vector2&) noexcept;
```

> Non-mutating `v * u` / `v / u` (vector × vector) are **not** provided.
> Use `v *= u` (mutating) or build a new vector explicitly.

### With a scalar

```cpp
Vector2& operator*=(const Type& v) noexcept;
Vector2& operator/=(const Type& v) noexcept;
Vector2& operator+=(const Type& v) noexcept;
Vector2& operator-=(const Type& v) noexcept;

constexpr Vector2 operator*(const Type& v) const noexcept;
constexpr Vector2 operator/(const Type& v) const noexcept;
```

### Binary (+/−) with another vector (non-mutating)

```cpp
constexpr Vector2 operator+(const Vector2&) const noexcept;
constexpr Vector2 operator-(const Vector2&) const noexcept;
```

### Unary

```cpp
constexpr Vector2 operator-() const noexcept;   // negation
```

---

## Geometry & helpers

```cpp
Type        distance_to      (const Vector2&) const noexcept;     // sqrt of squared distance
constexpr Type distance_to_sqr(const Vector2&) const noexcept;

constexpr Type dot(const Vector2&) const noexcept;

#ifndef _MSC_VER
constexpr Type   length() const noexcept;        // uses std::hypot; constexpr on non-MSVC
constexpr Vector2 normalized() const noexcept;   // returns *this if length==0
#else
Type   length() const noexcept;
Vector2 normalized() const noexcept;
#endif

constexpr Type   length_sqr() const noexcept;    // x*x + y*y
Vector2&         abs() noexcept;                 // component-wise absolute (constexpr-friendly impl)

constexpr Type   sum() const noexcept;           // x + y
constexpr std::tuple<Type, Type> as_tuple() const noexcept;
```

---

## ImGui integration (optional)

Define `OMATH_IMGUI_INTEGRATION` **before** including the header.

```cpp
#ifdef OMATH_IMGUI_INTEGRATION
constexpr ImVec2 to_im_vec2() const noexcept;         // {float(x), float(y)}
static Vector2   from_im_vec2(const ImVec2&) noexcept;
#endif
```

---

## Hashing & formatting

* **Hash (for `Vector2<float>`)**

  ```cpp
  template<> struct std::hash<omath::Vector2<float>> {
      std::size_t operator()(const omath::Vector2<float>&) const noexcept;
  };
  ```

  Example:

  ```cpp
  std::unordered_set<omath::Vector2<float>> set;
  set.insert({1.f, 2.f});
  ```

* **`std::formatter`** (for any `Type`)

  ```cpp
  // prints "[x, y]" for char / wchar_t / char8_t
  template<class Type>
  struct std::formatter<omath::Vector2<Type>>;
  ```

---

## Notes & invariants

* `Type` must be arithmetic (e.g., `float`, `double`, `int`, …).
* `normalized()` returns the input unchanged if `length() == 0`.
* `abs()` uses a constexpr-friendly implementation (not `std::abs`) to allow compile-time evaluation.
* On MSVC, `length()`/`normalized()` are not `constexpr` due to library constraints; they’re still `noexcept`.

---

## Examples

### Component-wise operations and scalar scaling

```cpp
omath::Vector2<float> u{2, 3}, v{4, 5};

u += v;                // (6, 8)
u -= v;                // (2, 3)
u *= v;                // (8, 15)  Hadamard product (mutates u)
auto w = v * 2.0f;     // (8, 10)  non-mutating scalar multiply
```

### Geometry helpers

```cpp
omath::Vector2<double> p{0.0, 0.0}, q{3.0, 4.0};

auto dsq = p.distance_to_sqr(q);   // 25
auto d   = p.distance_to(q);       // 5
auto dot = p.dot(q);               // 0
auto uq  = q.normalized();         // (0.6, 0.8)
```

### Using as a key in unordered containers (`float`)

```cpp
std::unordered_map<omath::Vector2<float>, int> counts;
counts[{1.f, 2.f}] = 42;
```

### ImGui interop

```cpp
#define OMATH_IMGUI_INTEGRATION
#include "vector2.hpp"

omath::Vector2<float> v{10, 20};
ImVec2 iv = v.to_im_vec2();
v = omath::Vector2<float>::from_im_vec2(iv);
```

---

## API summary (signatures)

```cpp
// Constructors
constexpr Vector2();
constexpr Vector2(const Type& x, const Type& y) noexcept;

// Equality & ordering
constexpr bool operator==(const Vector2&) const noexcept;
constexpr bool operator!=(const Vector2&) const noexcept;
bool operator< (const Vector2&) const noexcept;
bool operator> (const Vector2&) const noexcept;
bool operator<=(const Vector2&) const noexcept;
bool operator>=(const Vector2&) const noexcept;

// Compound (vector/vector and scalar)
Vector2& operator+=(const Vector2&) noexcept;
Vector2& operator-=(const Vector2&) noexcept;
Vector2& operator*=(const Vector2&) noexcept;
Vector2& operator/=(const Vector2&) noexcept;
Vector2& operator*=(const Type&) noexcept;
Vector2& operator/=(const Type&) noexcept;
Vector2& operator+=(const Type&) noexcept;
Vector2& operator-=(const Type&) noexcept;

// Non-mutating arithmetic
constexpr Vector2 operator+(const Vector2&) const noexcept;
constexpr Vector2 operator-(const Vector2&) const noexcept;
constexpr Vector2 operator*(const Type&) const noexcept;
constexpr Vector2 operator/(const Type&) const noexcept;
constexpr Vector2 operator-() const noexcept;

// Geometry
Type        distance_to(const Vector2&) const noexcept;
constexpr Type distance_to_sqr(const Vector2&) const noexcept;
constexpr Type dot(const Vector2&) const noexcept;
Type        length() const noexcept;                // constexpr on non-MSVC
Vector2     normalized() const noexcept;           // constexpr on non-MSVC
constexpr Type length_sqr() const noexcept;
Vector2&    abs() noexcept;
constexpr Type sum() const noexcept;
constexpr std::tuple<Type,Type> as_tuple() const noexcept;

// ImGui (optional)
#ifdef OMATH_IMGUI_INTEGRATION
constexpr ImVec2 to_im_vec2() const noexcept;
static Vector2   from_im_vec2(const ImVec2&) noexcept;
#endif

// Hash (float) and formatter are specialized in the header
```

---

## See Also

- [Vector3 Documentation](vector3.md) - 3D vector operations
- [Vector4 Documentation](vector4.md) - 4D vector operations
- [Getting Started Guide](../getting_started.md) - Quick start with OMath
- [Tutorials](../tutorials.md) - Step-by-step examples

---

*Last updated: 24 Dec 2025*
