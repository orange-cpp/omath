# FAQ

Common questions and answers about OMath.

---

## General Questions

### What is OMath?

OMath is a modern C++ math library designed for game development, graphics programming, and high-performance computing. It provides:
- Vector and matrix operations
- 3D projection and camera systems
- Projectile prediction
- Collision detection
- Support for multiple game engines (Source, Unity, Unreal, etc.)
- Pattern scanning utilities

### Why choose OMath over other math libraries?

- **Modern C++**: Uses C++20/23 features (concepts, `constexpr`, `std::expected`)
- **No legacy code**: Built from scratch without legacy baggage
- **Game engine support**: Pre-configured for Source, Unity, Unreal, Frostbite, etc.
- **Zero dependencies**: No external dependencies needed (except for testing)
- **Performance**: AVX2 optimizations available
- **Type safety**: Strong typing prevents common errors
- **Cross-platform**: Works on Windows, Linux, and macOS

### Is OMath suitable for production use?

Yes! OMath is production-ready and used in various projects. It has:
- Comprehensive test coverage
- Clear error handling
- Well-documented API
- Active maintenance and community support

---

## Installation & Setup

### How do I install OMath?

Three main methods:

**vcpkg (recommended):**
```bash
vcpkg install orange-math
```

**xrepo:**
```bash
xrepo install omath
```

**From source:**
See [Installation Guide](install.md)

### What are the minimum requirements?

- **Compiler**: C++20 support required
  - GCC 10+
  - Clang 11+
  - MSVC 2019 16.10+
- **CMake**: 3.15+ (if building from source)
- **Platform**: Windows, Linux, or macOS

### Do I need C++23?

C++23 is **recommended** but not required. Some features like `std::expected` work better with C++23, but fallbacks are available for C++20.

### Can I use OMath in a C++17 project?

No, OMath requires C++20 minimum due to use of concepts, `constexpr` enhancements, and other C++20 features.

---

## Usage Questions

### How do I include OMath in my project?

**Full library:**
```cpp
#include <omath/omath.hpp>
```

**Specific components:**
```cpp
#include <omath/linear_algebra/vector3.hpp>
#include <omath/engines/source_engine/camera.hpp>
```

### Which game engine should I use?

Choose based on your target game or application:

| Engine | Use For |
|--------|---------|
| **Source Engine** | CS:GO, TF2, CS2, Half-Life, Portal, L4D |
| **Unity Engine** | Unity games (many indie and mobile games) |
| **Unreal Engine** | Fortnite, Unreal games |
| **Frostbite** | Battlefield, Star Wars games (EA titles) |
| **IW Engine** | Call of Duty series |
| **OpenGL** | Custom OpenGL applications, generic 3D |

### How do I switch between engines?

Just change the namespace:

```cpp
// Source Engine
using namespace omath::source_engine;
Camera cam = /* ... */;

// Unity Engine
using namespace omath::unity_engine;
Camera cam = /* ... */;
```

Each engine has the same API but different coordinate system handling.

### What if my game isn't listed?

Use the **OpenGL engine** as a starting point - it uses canonical OpenGL conventions. You may need to adjust coordinate transformations based on your specific game.

---

## Performance Questions

### Should I use the AVX2 or Legacy engine?

**Use AVX2 if:**
- Target modern CPUs (2013+)
- Need maximum performance
- Can accept reduced compatibility

**Use Legacy if:**
- Need broad compatibility
- Target older CPUs or ARM
- Unsure about target hardware

The API is identical - just change the class:
```cpp
// Legacy (compatible)
ProjPredEngineLegacy engine;

// AVX2 (faster)
ProjPredEngineAVX2 engine;
```

### How much faster is AVX2?

Typically 2-4x faster for projectile prediction calculations, depending on the CPU and specific use case.

### Are vector operations constexpr?

Yes! Most operations are `constexpr` and can be evaluated at compile-time:

```cpp
constexpr Vector3<float> v{1, 2, 3};
constexpr auto len_sq = v.length_sqr();  // Computed at compile time
```

### Is OMath thread-safe?

- **Immutable operations** (vector math, etc.) are thread-safe
- **Mutable state** (Camera updates) is NOT thread-safe
- Use separate instances per thread or synchronize access

---

## Troubleshooting

### `world_to_screen()` always returns `nullopt`

Check:
1. **Is the point behind the camera?** Points behind the camera cannot be projected.
2. **Are near/far planes correct?** Ensure `near < far` and both are positive.
3. **Is FOV valid?** FOV should be between 1° and 179°.
4. **Are camera angles normalized?** Use engine-provided angle types.

### Angles are wrapping incorrectly

Use the correct angle type:
```cpp
// Good: uses proper angle type
PitchAngle pitch = PitchAngle::from_degrees(45.0f);

// Bad: raw float loses normalization
float pitch = 45.0f;
```

### Projection seems mirrored or inverted

You may be using the wrong engine trait. Each engine has different coordinate conventions:
- **Source/Unity**: Z-up
- **Unreal**: Z-up, different handedness
- **OpenGL**: Y-up

Ensure you're using the trait matching your game.

### Pattern scanning finds multiple matches

This is normal! Patterns may appear multiple times. Solutions:
1. Make the pattern more specific (more bytes, fewer wildcards)
2. Use additional context (nearby code patterns)
3. Verify each match programmatically

### Projectile prediction returns `nullopt`

Common reasons:
1. **Target too fast**: Target velocity exceeds projectile speed
2. **Out of range**: Distance exceeds max flight time
3. **Invalid input**: Check projectile speed > 0
4. **Gravity too strong**: Projectile can't reach target height

### Compilation errors about `std::expected`

If using C++20 (not C++23), you may need a backport library like `tl::expected`:

```cmake
# CMakeLists.txt
find_package(tl-expected CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE tl::expected)
```

Or upgrade to C++23 if possible.

---

## Feature Questions

### Can I use OMath with DirectX/OpenGL/Vulkan?

Yes! OMath matrices and vectors work with all graphics APIs. Use:
- **OpenGL**: `opengl_engine` traits
- **DirectX**: Use appropriate engine trait or OpenGL as base
- **Vulkan**: Use OpenGL traits as starting point

### Does OMath support quaternions?

Not currently. Quaternion support may be added in future versions. For now, use euler angles (ViewAngles) or convert manually.

### Can I extend OMath with custom engine traits?

Yes! Implement the `CameraEngineConcept`:

```cpp
class MyEngineTrait {
public:
    static ViewAngles calc_look_at_angle(
        const Vector3<float>& origin,
        const Vector3<float>& target
    );
    
    static Mat4X4 calc_view_matrix(
        const ViewAngles& angles,
        const Vector3<float>& origin
    );
    
    static Mat4X4 calc_projection_matrix(
        const FieldOfView& fov,
        const ViewPort& viewport,
        float near, float far
    );
};

// Use with Camera
using MyCamera = Camera<Mat4X4, ViewAngles, MyEngineTrait>;
```

### Does OMath support SIMD for vector operations?

AVX2 support is available for projectile prediction. General vector SIMD may be added in future versions. The library already compiles to efficient code with compiler optimizations enabled.

### Can I use OMath for machine learning?

OMath is optimized for game development and graphics, not ML. For machine learning, consider libraries like Eigen or xtensor which are designed for that domain.

---

## Debugging Questions

### How do I print vectors?

OMath provides `std::formatter` support:

```cpp
#include <format>
#include <iostream>

Vector3<float> v{1, 2, 3};
std::cout << std::format("{}", v) << "\n";  // Prints: [1, 2, 3]
```

### How do I visualize projection problems?

1. Check if `world_to_screen()` succeeds
2. Print camera matrices:
   ```cpp
   auto view = camera.get_view_matrix();
   auto proj = camera.get_projection_matrix();
   // Print matrix values
   ```
3. Test with known good points (e.g., origin, simple positions)
4. Verify viewport and FOV values

### How can I debug pattern scanning?

```cpp
PatternView pattern{"48 8B 05 ?? ?? ?? ??"};

// Print pattern details
std::cout << "Pattern length: " << pattern.size() << "\n";
std::cout << "Pattern bytes: ";
for (auto byte : pattern) {
    if (byte.has_value()) {
        std::cout << std::hex << (int)*byte << " ";
    } else {
        std::cout << "?? ";
    }
}
std::cout << "\n";
```

---

## Contributing

### How can I contribute to OMath?

See [CONTRIBUTING.md](https://github.com/orange-cpp/omath/blob/master/CONTRIBUTING.md) for guidelines. Contributions welcome:
- Bug fixes
- New features
- Documentation improvements
- Test coverage
- Examples

### Where do I report bugs?

[GitHub Issues](https://github.com/orange-cpp/omath/issues)

Please include:
- OMath version
- Compiler and version
- Minimal reproducible example
- Expected vs actual behavior

### How do I request a feature?

Open a GitHub issue with:
- Use case description
- Proposed API (if applicable)
- Why existing features don't meet your needs

---

## License & Legal

### What license does OMath use?

OMath uses a custom "libomath" license. See [LICENSE](https://github.com/orange-cpp/omath/blob/master/LICENSE) for full details.

### Can I use OMath in commercial projects?

Check the LICENSE file for commercial use terms.

### Can I use OMath for game cheating/hacking?

OMath is a math library and can be used for various purposes. However:
- Using it to cheat in online games may violate game ToS
- Creating cheats may be illegal in your jurisdiction
- The developers do not condone cheating in online games

Use responsibly and ethically.

---

## Getting Help

### Where can I get help?

- **Documentation**: [http://libomath.org](http://libomath.org)
- **Discord**: [Join community](https://discord.gg/eDgdaWbqwZ)
- **Telegram**: [@orangennotes](https://t.me/orangennotes)
- **GitHub Issues**: [Report bugs/ask questions](https://github.com/orange-cpp/omath/issues)

### Is there a Discord/community?

Yes! Join our Discord: [https://discord.gg/eDgdaWbqwZ](https://discord.gg/eDgdaWbqwZ)

### Are there video tutorials?

Check our [YouTube channel](https://youtu.be/lM_NJ1yCunw?si=-Qf5yzDcWbaxAXGQ) for demonstrations and tutorials.

---

## Didn't find your answer?

- Search the [documentation](index.md)
- Check [tutorials](tutorials.md)
- Ask on [Discord](https://discord.gg/eDgdaWbqwZ)
- Open a [GitHub issue](https://github.com/orange-cpp/omath/issues)

---

*Last updated: 24 Dec 2025*
