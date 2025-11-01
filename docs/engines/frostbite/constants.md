Nice! A clean little “types + constants” header. A few quick fixes and polish:

## Issues / suggestions

1. **Mat3X3 alias is wrong**

* You wrote `using Mat3X3 = Mat<4, 4, float, MatStoreType::ROW_MAJOR>;`
* That should be `Mat<3, 3, ...>`.

2. **`constexpr` globals in a header → make them `inline constexpr`**

* Since this is in a header included by multiple TUs, use `inline constexpr` to avoid ODR/link issues (C++17+).

3. **Consider column-major vs row-major**

* Most game/graphics stacks (GLSL/HLSL, many engines) lean column-major and column vectors. If the rest of your math lib or shaders assume column-major, align these typedefs now to avoid silent transposes later. If row-major is intentional, all good—just be consistent.

4. **Naming consistency**

* If you prefer `k_` prefix, keep it; otherwise consider `kAbsUp`/`ABS_UP` to match your codebase’s conventions.

5. **`Mat1X3` as a “row vector”**

* If you actually use it as a 3-component vector, consider just `Vector3<float>` (clearer) and reserve `Mat1X3` for real row-vector math.

---

## Tidied version

```cpp
// Created by Vlad on 10/21/2025.
#pragma once

#include "omath/linear_algebra/mat.hpp"
#include "omath/linear_algebra/vector3.hpp"
#include <omath/trigonometry/angle.hpp>
#include <omath/trigonometry/view_angles.hpp>

namespace omath::frostbite_engine
{
    // Inline to avoid ODR across translation units
    inline constexpr Vector3<float> k_abs_up      = {0.0f, 1.0f, 0.0f};
    inline constexpr Vector3<float> k_abs_right   = {1.0f, 0.0f, 0.0f};
    inline constexpr Vector3<float> k_abs_forward = {0.0f, 0.0f, 1.0f};

    // NOTE: verify row/column major matches the rest of your engine
    using Mat4X4 = Mat<4, 4, float, MatStoreType::ROW_MAJOR>;
    using Mat3X3 = Mat<3, 3, float, MatStoreType::ROW_MAJOR>;
    using Mat1X3 = Mat<1, 3, float, MatStoreType::ROW_MAJOR>;

    using PitchAngle = Angle<float, -90.0f,  90.0f,  AngleFlags::Clamped   >;
    using YawAngle   = Angle<float,-180.0f, 180.0f,  AngleFlags::Normalized>;
    using RollAngle  = Angle<float,-180.0f, 180.0f,  AngleFlags::Normalized>;

    using ViewAngles = omath::ViewAngles<PitchAngle, YawAngle, RollAngle>;
} // namespace omath::frostbite_engine
```

If you share how your matrices multiply vectors (row vs column) and your world handedness, I can double-check the axis constants and angle normalization to make sure yaw/pitch signs line up with your camera and `atan2` usage.
