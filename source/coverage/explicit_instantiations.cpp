#include <omath/linear_algebra/vector3.hpp>
#include <omath/linear_algebra/vector4.hpp>
#include <omath/linear_algebra/triangle.hpp>

// Explicitly instantiate non-inlined helpers for float variants so that
// the compiler emits out-of-line definitions in this TU. This is an
// "honest" runtime approach because it produces real object code
// (no #line shims) that coverage tools can inspect.

#if defined(_MSC_VER)
template omath::Vector3<float>::ContainedType omath::Vector3<float>::length() const noexcept;
#else
template omath::Vector3<float>::ContainedType omath::Vector3<float>::length() const;
#endif
template std::expected<omath::Angle<float, 0.f, 180.f, omath::AngleFlags::Clamped>, omath::Vector3Error>
omath::Vector3<float>::angle_between(const omath::Vector3<float>& other) const noexcept;
template bool omath::Vector3<float>::is_perpendicular(const omath::Vector3<float>& other, omath::Vector3<float>::ContainedType) const noexcept;
#if defined(_MSC_VER)
template omath::Vector3<float> omath::Vector3<float>::normalized() const noexcept;
#else
template omath::Vector3<float> omath::Vector3<float>::normalized() const;
#endif

template omath::Vector4<float>::ContainedType omath::Vector4<float>::length() const noexcept;
template omath::Vector4<float>::ContainedType omath::Vector4<float>::sum() const noexcept;
template omath::Vector4<float>& omath::Vector4<float>::clamp(const omath::Vector4<float>::ContainedType& min, const omath::Vector4<float>::ContainedType& max) noexcept;

template omath::Vector3<float> omath::Triangle<omath::Vector3<float>>::calculate_normal() const;
template omath::Vector3<float>::ContainedType omath::Triangle<omath::Vector3<float>>::side_a_length() const;
template omath::Vector3<float>::ContainedType omath::Triangle<omath::Vector3<float>>::side_b_length() const;
template omath::Vector3<float>::ContainedType omath::Triangle<omath::Vector3<float>>::hypot() const;
template bool omath::Triangle<omath::Vector3<float>>::is_rectangular() const;
