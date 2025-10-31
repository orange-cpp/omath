# `omath::Vector4` — 4D vector (C++20/23)

> Header: your project’s `vector4.hpp`
> Namespace: `omath`
> Template: `template<class Type> requires std::is_arithmetic_v<Type>`
> Inherits: `omath::Vector3<Type>` (brings `x`, `y`, `z` and most scalar ops)

`Vector4<Type>` extends `Vector3<Type>` with a `w` component and 4D operations: component-wise arithmetic, scalar ops, dot/length helpers, clamping, hashing (for `float`) and `std::formatter` support. Optional ImGui interop is available behind a macro.

---

## Quick start

```cpp
#include "vector4.hpp"
using omath::Vector4;

using Vec4f = Vector4<float>;

Vec4f a{1, 2, 3, 1};
Vec4f b{4, 5, 6, 2};

// Component-wise & scalar ops
auto c  = a + b;         // (5, 7, 9, 3)
c      *= 0.5f;          // (2.5, 3.5, 4.5, 1.5)
auto h  = a * b;         // Hadamard: (4, 10, 18, 2)

// Dot / length
float d  = a.dot(b);     // 1*4 + 2*5 + 3*6 + 1*2 = 32
float L  = a.length();   // sqrt(x²+y²+z²+w²)

// Clamp (x,y,z only; see notes)
Vec4f col{1.4f, -0.2f, 0.7f, 42.f};
col.clamp(0.f, 1.f);     // -> (1, 0, 0.7, w unchanged)
```

---

## Data members

```cpp
// Inherited from Vector3<Type>:
Type x{0};
Type y{0};
Type z{0};

// Added in Vector4:
Type w{0};
```

---

## Constructors

```cpp
constexpr Vector4() noexcept;                                      // (0,0,0,0)
constexpr Vector4(const Type& x, const Type& y,
                  const Type& z, const Type& w);                    // value-init
```

---

## Equality & ordering

```cpp
constexpr bool operator==(const Vector4&) const noexcept;           // component-wise
constexpr bool operator!=(const Vector4&) const noexcept;

bool operator< (const Vector4&) const noexcept; // compare by length()
bool operator> (const Vector4&) const noexcept;
bool operator<=(const Vector4&) const noexcept;
bool operator>=(const Vector4&) const noexcept;
```

> **Note:** Ordering uses **magnitude** (Euclidean norm), not lexicographic order.

---

## Arithmetic (mutating)

With another vector (component-wise):

```cpp
Vector4& operator+=(const Vector4&) noexcept;
Vector4& operator-=(const Vector4&) noexcept;
Vector4& operator*=(const Vector4&) noexcept;   // Hadamard
Vector4& operator/=(const Vector4&) noexcept;
```

With a scalar:

```cpp
Vector4& operator*=(const Type& v) noexcept;
Vector4& operator/=(const Type& v) noexcept;

// From base class (inherited):
Vector4& operator+=(const Type& v) noexcept;    // adds v to x,y,z  (and w via base? see notes)
Vector4& operator-=(const Type& v) noexcept;
```

---

## Arithmetic (non-mutating)

```cpp
constexpr Vector4 operator-()                      const noexcept;
constexpr Vector4 operator+(const Vector4&)        const noexcept;
constexpr Vector4 operator-(const Vector4&)        const noexcept;
constexpr Vector4 operator*(const Vector4&)        const noexcept;   // Hadamard
constexpr Vector4 operator/(const Vector4&)        const noexcept;   // Hadamard
constexpr Vector4 operator*(const Type& scalar)    const noexcept;
constexpr Vector4 operator/(const Type& scalar)    const noexcept;
```

---

## Geometry & helpers

```cpp
constexpr Type length_sqr()   const noexcept;     // x² + y² + z² + w²
Type           length()       const noexcept;     // std::sqrt(length_sqr())
constexpr Type dot(const Vector4& rhs) const noexcept;

Vector4&       abs() noexcept;                    // component-wise absolute
Vector4&       clamp(const Type& min, const Type& max) noexcept;
                                                  // clamps x,y,z; leaves w unchanged (see notes)
constexpr Type sum()          const noexcept;     // x + y + z + w
```

---

## ImGui integration (optional)

Guarded by `OMATH_IMGUI_INTEGRATION`:

```cpp
#ifdef OMATH_IMGUI_INTEGRATION
constexpr ImVec4 to_im_vec4() const noexcept;
// NOTE: Provided signature returns Vector4<float> and (in current code) sets only x,y,z.
// See "Notes & caveats" for a corrected version you may prefer.
static Vector4<float> from_im_vec4(const ImVec4& other) noexcept;
#endif
```

---

## Hashing & formatting

* **Hash specialization** (only for `Vector4<float>`):

  ```cpp
  template<> struct std::hash<omath::Vector4<float>> {
    std::size_t operator()(const omath::Vector4<float>&) const noexcept;
  };
  ```

  Example:

  ```cpp
  std::unordered_map<omath::Vector4<float>, int> counts;
  counts[{1.f, 2.f, 3.f, 1.f}] = 7;
  ```

* **`std::formatter`** (for any `Type`, all character kinds):

  ```cpp
  template<class Type>
  struct std::formatter<omath::Vector4<Type>>;   // -> "[x, y, z, w]"
  ```

---

## Notes & caveats (as implemented)

* `clamp(min,max)` **clamps only `x`, `y`, `z`** and **does not clamp `w`**. This may be intentional (e.g., when `w` is a homogeneous coordinate) — document your intent in your codebase.
  If you want to clamp `w` too:

  ```cpp
  w = std::clamp(w, min, max);
  ```

* **ImGui interop**:

    * The header references `ImVec4` but does not include `<imgui.h>` itself. Ensure it’s included **before** this header whenever `OMATH_IMGUI_INTEGRATION` is defined.
    * `from_im_vec4` currently returns `Vector4<float>` and (in the snippet shown) initializes **only x,y,z**. A more consistent version would be:

      ```cpp
      #ifdef OMATH_IMGUI_INTEGRATION
      static Vector4<Type> from_im_vec4(const ImVec4& v) noexcept {
        return {static_cast<Type>(v.x), static_cast<Type>(v.y),
                static_cast<Type>(v.z), static_cast<Type>(v.w)};
      }
      #endif
      ```

* Many scalar compound operators (`+= Type`, `-= Type`) are inherited from `Vector3<Type>`.

---

## API summary (signatures)

```cpp
// Ctors
constexpr Vector4() noexcept;
constexpr Vector4(const Type& x, const Type& y, const Type& z, const Type& w);

// Equality & ordering
constexpr bool operator==(const Vector4&) const noexcept;
constexpr bool operator!=(const Vector4&) const noexcept;
bool operator< (const Vector4&) const noexcept;
bool operator> (const Vector4&) const noexcept;
bool operator<=(const Vector4&) const noexcept;
bool operator>=(const Vector4&) const noexcept;

// Mutating arithmetic
Vector4& operator+=(const Vector4&) noexcept;
Vector4& operator-=(const Vector4&) noexcept;
Vector4& operator*=(const Vector4&) noexcept;
Vector4& operator/=(const Vector4&) noexcept;
Vector4& operator*=(const Type&)   noexcept;
Vector4& operator/=(const Type&)   noexcept;
// (inherited) Vector4& operator+=(const Type&) noexcept;
// (inherited) Vector4& operator-=(const Type&) noexcept;

// Non-mutating arithmetic
constexpr Vector4 operator-() const noexcept;
constexpr Vector4 operator+(const Vector4&) const noexcept;
constexpr Vector4 operator-(const Vector4&) const noexcept;
constexpr Vector4 operator*(const Vector4&) const noexcept;
constexpr Vector4 operator/(const Vector4&) const noexcept;
constexpr Vector4 operator*(const Type&)    const noexcept;
constexpr Vector4 operator/(const Type&)    const noexcept;

// Geometry & helpers
constexpr Type length_sqr() const noexcept;
Type           length()     const noexcept;
constexpr Type dot(const Vector4&) const noexcept;
Vector4&       abs() noexcept;
Vector4&       clamp(const Type& min, const Type& max) noexcept;
constexpr Type sum() const noexcept;

// ImGui (optional)
#ifdef OMATH_IMGUI_INTEGRATION
constexpr ImVec4 to_im_vec4() const noexcept;
static Vector4<float> from_im_vec4(const ImVec4&) noexcept; // see note for preferred template version
#endif

// Hash (float) and formatter specializations provided below the class
```

---

*Last updated: 31 Oct 2025*
