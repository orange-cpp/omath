# `omath::Mat` — Matrix class (C++20/23)

> Header: your project’s `mat.hpp` (requires `vector3.hpp`)
> Namespace: `omath`
> Requires: **C++23** (uses multi-parameter `operator[]`)
> SIMD (optional): define **`OMATH_USE_AVX2`** to enable AVX2-accelerated multiplication for `float`/`double`.

---

## Overview

`omath::Mat<Rows, Columns, Type, StoreType>` is a compile-time, fixed-size matrix with:

* **Row/column counts** as template parameters (no heap allocations).
* **Row-major** or **column-major** storage (compile-time via `MatStoreType`).
* **Arithmetic** and **linear algebra**: matrix × matrix, scalar ops, transpose, determinant, inverse (optional), etc.
* **Transform helpers**: translation, axis rotations, look-at, perspective & orthographic projections.
* **I/O helpers**: `to_string`/`to_wstring`/`to_u8string` and `std::formatter` specializations.

---

## Template parameters

| Parameter   | Description                                                              | Default     |
| ----------- | ------------------------------------------------------------------------ | ----------- |
| `Rows`      | Number of rows (size_t, compile-time)                                    | —           |
| `Columns`   | Number of columns (size_t, compile-time)                                 | —           |
| `Type`      | Element type (arithmetic)                                                | `float`     |
| `StoreType` | Storage order: `MatStoreType::ROW_MAJOR` or `MatStoreType::COLUMN_MAJOR` | `ROW_MAJOR` |

```cpp
enum class MatStoreType : uint8_t { ROW_MAJOR = 0, COLUMN_MAJOR };
```

---

## Quick start

```cpp
#include "mat.hpp"
using omath::Mat;

// 4x4 float, row-major
Mat<4,4> I = {
  {1,0,0,0},
  {0,1,0,0},
  {0,0,1,0},
  {0,0,0,1},
};

// Multiply 4x4 transforms
Mat<4,4> A = { {1,2,3,0},{0,1,4,0},{5,6,0,0},{0,0,0,1} };
Mat<4,4> B = { {2,0,0,0},{0,2,0,0},{0,0,2,0},{0,0,0,1} };
Mat<4,4> C = A * B;                   // matrix × matrix

// Scalar ops
auto D = C * 0.5f;                    // scale all entries

// Indexing (C++23 multi-parameter operator[])
float a03 = A[0,3];                   // same as A.at(0,3)
A[1,2] = 42.0f;

// Transpose, determinant, inverse
auto AT = A.transposed();
float det = A.determinant();          // only for square matrices
auto inv = A.inverted();              // std::optional<Mat>; std::nullopt if non-invertible
```

> **Note**
> Multiplication requires the **same** `StoreType` and `Type` on both operands, and dimensions must match at compile time.

---

## Construction

```cpp
Mat();                                 // zero-initialized
Mat(std::initializer_list<std::initializer_list<Type>> rows);
explicit Mat(const Type* raw_data);    // copies Rows*Columns elements
Mat(const Mat&); Mat(Mat&&);
```

* **Zeroing/setting**

  ```cpp
  m.clear();            // set all entries to 0
  m.set(3.14f);         // set all entries to a value
  ```

* **Shape & metadata**

  ```cpp
  Mat<>::row_count();             // constexpr size_t
  Mat<>::columns_count();         // constexpr size_t
  Mat<>::size();                  // constexpr MatSize {rows, columns}
  Mat<>::get_store_ordering();    // constexpr MatStoreType
  using ContainedType = Type;     // alias
  ```

---

## Element access

```cpp
T&       at(size_t r, size_t c);
T const& at(size_t r, size_t c) const;

T&       operator[](size_t r, size_t c);       // C++23
T const& operator[](size_t r, size_t c) const; // C++23
```

> **Bounds checking**
> In debug builds you may enable/disable range checks via your compile-time macros (see the source guard around `at()`).

---

## Arithmetic

* **Matrix × matrix**

  ```cpp
  // (Rows x Columns) * (Columns x OtherColumns) -> (Rows x OtherColumns)
  template<size_t OtherColumns>
  Mat<Rows, OtherColumns, Type, StoreType>
  operator*(const Mat<Columns, OtherColumns, Type, StoreType>&) const;
  ```

    * Complexity: `O(Rows * Columns * OtherColumns)`.
    * AVX2-accelerated when `OMATH_USE_AVX2` is defined and `Type` is `float` or `double`.

* **Scalars**

  ```cpp
  Mat  operator*(const Type& s) const;  Mat& operator*=(const Type& s);
  Mat  operator/(const Type& s) const;  Mat& operator/=(const Type& s);
  ```

* **Transpose**

  ```cpp
  Mat<Columns, Rows, Type, StoreType> transposed() const noexcept;
  ```

* **Determinant (square only)**

  ```cpp
  Type determinant() const;  // 1x1, 2x2 fast path; larger uses Laplace expansion
  ```

* **Inverse (square only)**

  ```cpp
  std::optional<Mat> inverted() const;  // nullopt if det == 0
  ```

* **Minors & cofactors (square only)**

  ```cpp
  Mat<Rows-1, Columns-1, Type, StoreType> strip(size_t r, size_t c) const;
  Type minor(size_t r, size_t c) const;
  Type alg_complement(size_t r, size_t c) const; // cofactor
  ```

* **Utilities**

  ```cpp
  Type sum() const noexcept;
  auto& raw_array();               // std::array<Type, Rows*Columns>&
  auto const& raw_array() const;
  ```

* **Comparison / formatting**

  ```cpp
  bool operator==(const Mat&) const;
  bool operator!=(const Mat&) const;

  std::string  to_string()  const noexcept;
  std::wstring to_wstring() const noexcept;
  std::u8string to_u8string() const noexcept;
  ```

// std::formatter specialization provided for char, wchar_t, char8_t

````

---

## Storage order notes

- **Row-major**: `index = row * Columns + column`
- **Column-major**: `index = row + column * Rows`

Choose one **consistently** across your math types and shader conventions. Mixed orders are supported by the type system but not for cross-multiplying (store types must match).

---

## Transform helpers

### From vectors

```cpp
template<class T=float, MatStoreType St=ROW_MAJOR>
Mat<1,4,T,St> mat_row_from_vector(const Vector3<T>& v);

template<class T=float, MatStoreType St=ROW_MAJOR>
Mat<4,1,T,St> mat_column_from_vector(const Vector3<T>& v);
````

### Translation

```cpp
template<class T=float, MatStoreType St=ROW_MAJOR>
Mat<4,4,T,St> mat_translation(const Vector3<T>& d) noexcept;
```

### Axis rotations

```cpp
// Angle type must provide angle.cos() and angle.sin()
template<class T=float, MatStoreType St=ROW_MAJOR, class Angle>
Mat<4,4,T,St> mat_rotation_axis_x(const Angle& a) noexcept;

template<class T=float, MatStoreType St=ROW_MAJOR, class Angle>
Mat<4,4,T,St> mat_rotation_axis_y(const Angle& a) noexcept;

template<class T=float, MatStoreType St=ROW_MAJOR, class Angle>
Mat<4,4,T,St> mat_rotation_axis_z(const Angle& a) noexcept;
```

### Camera/view

```cpp
template<class T=float, MatStoreType St=ROW_MAJOR>
Mat<4,4,T,St> mat_camera_view(const Vector3<T>& forward,
                              const Vector3<T>& right,
                              const Vector3<T>& up,
                              const Vector3<T>& camera_origin) noexcept;
```

### Perspective projections

```cpp
template<class T=float, MatStoreType St=ROW_MAJOR>
Mat<4,4,T,St> mat_perspective_left_handed (float fov_deg, float aspect, float near, float far) noexcept;

template<class T=float, MatStoreType St=ROW_MAJOR>
Mat<4,4,T,St> mat_perspective_right_handed(float fov_deg, float aspect, float near, float far) noexcept;
```

### Orthographic projections

```cpp
template<class T=float, MatStoreType St=ROW_MAJOR>
Mat<4,4,T,St> mat_ortho_left_handed (T left, T right, T bottom, T top, T near, T far) noexcept;

template<class T=float, MatStoreType St=ROW_MAJOR>
Mat<4,4,T,St> mat_ortho_right_handed(T left, T right, T bottom, T top, T near, T far) noexcept;
```

### Look-at matrices

```cpp
template<class T=float, MatStoreType St=COLUMN_MAJOR>
Mat<4,4,T,St> mat_look_at_left_handed (const Vector3<T>& eye,
                                       const Vector3<T>& center,
                                       const Vector3<T>& up);

template<class T=float, MatStoreType St=COLUMN_MAJOR>
Mat<4,4,T,St> mat_look_at_right_handed(const Vector3<T>& eye,
                                       const Vector3<T>& center,
                                       const Vector3<T>& up);
```

---

## Screen-space helper

```cpp
template<class Type=float>
static constexpr Mat<4,4> to_screen_mat(const Type& screen_w, const Type& screen_h) noexcept;
// Maps NDC to screen space (origin top-left, y down)
```

---

## Examples

### 1) Building a left-handed camera and perspective

```cpp
using V3 = omath::Vector3<float>;
using M4 = omath::Mat<4,4,float, omath::MatStoreType::COLUMN_MAJOR>;

V3 eye{0, 1, -5}, center{0, 0, 0}, up{0, 1, 0};
M4 view = omath::mat_look_at_left_handed<float, omath::MatStoreType::COLUMN_MAJOR>(eye, center, up);

float fov = 60.f, aspect = 16.f/9.f, n = 0.1f, f = 100.f;
M4 proj = omath::mat_perspective_left_handed<float, omath::MatStoreType::COLUMN_MAJOR>(fov, aspect, n, f);

// final VP
M4 vp = proj * view;
```

### 2) Inverting a transform safely

```cpp
omath::Mat<4,4> T = omath::mat_translation(omath::Vector3<float>{2,3,4});
if (auto inv = T.inverted()) {
  // use *inv
} else {
  // handle non-invertible
}
```

### 3) Formatting for logs

```cpp
omath::Mat<2,2> A = { {1,2},{3,4} };
std::string s = A.to_string();       // "[[    1.000,     2.000]\n [    3.000,     4.000]]"
std::string f = std::format("A = {}", A);  // uses std::formatter
```

---

## Performance

* **Cache-friendly kernels** per storage order when AVX2 is not enabled.
* **AVX2 path** (`OMATH_USE_AVX2`) for `float`/`double` implements FMAs with 256-bit vectors for both row-major and column-major multiplication.
* Complexity for `A(R×K) * B(K×C)`: **O(RKC)** regardless of storage order.

---

## Constraints & concepts

```cpp
template<typename M1, typename M2>
concept MatTemplateEqual =
  (M1::rows == M2::rows) &&
  (M1::columns == M2::columns) &&
  std::is_same_v<typename M1::value_type, typename M2::value_type> &&
  (M1::store_type == M2::store_type);
```

> Use this concept to constrain generic functions that operate on like-shaped matrices.

---

## Exceptions

* `std::invalid_argument` — initializer list dimensions mismatch.
* `std::out_of_range` — out-of-bounds in `at()` when bounds checking is active (see source guard).
* `inverted()` does **not** throw; returns `std::nullopt` if `determinant() == 0`.

---

## Build switches

* **`OMATH_USE_AVX2`** — enable AVX2 vectorized multiplication paths (`<immintrin.h>` required).
* **Debug checks** — the `at()` method contains a conditional range check; refer to the preprocessor guard in the code to enable/disable in your configuration.

---

## Known requirements & interoperability

* **C++23** is required for multi-parameter `operator[]`. If you target pre-C++23, use `at(r,c)` instead.
* All binary operations require matching `Type` and `StoreType`. Convert explicitly if needed.

---

## See also

* `omath::Vector3<T>`
* Projection helpers: `mat_perspective_*`, `mat_ortho_*`
* View helpers: `mat_look_at_*`, `mat_camera_view`
* Construction helpers: `mat_row_from_vector`, `mat_column_from_vector`, `mat_translation`, `mat_rotation_axis_*`

---

## Appendix: API summary (signatures)

```cpp
// Core
Mat(); Mat(const Mat&); Mat(Mat&&);
Mat(std::initializer_list<std::initializer_list<Type>>);
explicit Mat(const Type* raw);
Mat& operator=(const Mat&); Mat& operator=(Mat&&);

static constexpr size_t row_count();
static constexpr size_t columns_count();
static consteval MatSize size();
static constexpr MatStoreType get_store_ordering();

T& at(size_t r, size_t c);
T const& at(size_t r, size_t c) const;
T& operator[](size_t r, size_t c);
T const& operator[](size_t r, size_t c) const;

void clear();
void set(const Type& v);
Type sum() const noexcept;

template<size_t OC> Mat<Rows,OC,Type,StoreType> operator*(const Mat<Columns,OC,Type,StoreType>&) const;
Mat& operator*=(const Type&); Mat operator*(const Type&) const;
Mat& operator/=(const Type&); Mat operator/(const Type&) const;

Mat<Columns,Rows,Type,StoreType> transposed() const noexcept;
Type determinant() const;                              // square only
std::optional<Mat> inverted() const;                   // square only

Mat<Rows-1,Columns-1,Type,StoreType> strip(size_t r, size_t c) const;
Type minor(size_t r, size_t c) const;
Type alg_complement(size_t r, size_t c) const;

auto& raw_array(); auto const& raw_array() const;
std::string  to_string() const noexcept;
std::wstring to_wstring() const noexcept;
std::u8string to_u8string() const noexcept;

bool operator==(const Mat&) const;
bool operator!=(const Mat&) const;

// Helpers (see sections above)
```

---

*Last updated: 31 Oct 2025*
