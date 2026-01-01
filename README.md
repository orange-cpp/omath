<div align = center>

![banner](docs/images/logos/omath_logo_macro.png)

![GitHub License](https://img.shields.io/github/license/orange-cpp/omath)
![GitHub contributors](https://img.shields.io/github/contributors/orange-cpp/omath)
![GitHub top language](https://img.shields.io/github/languages/top/orange-cpp/omath)
![GitHub repo size](https://img.shields.io/github/repo-size/orange-cpp/omath)
[![CodeFactor](https://www.codefactor.io/repository/github/orange-cpp/omath/badge)](https://www.codefactor.io/repository/github/orange-cpp/omath)
![GitHub Actions Workflow Status](https://img.shields.io/github/actions/workflow/status/orange-cpp/omath/cmake-multi-platform.yml)
[![Vcpkg package](https://repology.org/badge/version-for-repo/vcpkg/orange-math.svg)](https://repology.org/project/orange-math/versions)
![GitHub forks](https://img.shields.io/github/forks/orange-cpp/omath)
[![discord badge](https://dcbadge.limes.pink/api/server/https://discord.gg/eDgdaWbqwZ?style=flat)](https://discord.gg/eDgdaWbqwZ)
[![telegram badge](https://img.shields.io/badge/Telegram-2CA5E0?style=flat-squeare&logo=telegram&logoColor=white)](https://t.me/orangennotes)

OMath is a 100% independent, constexpr template blazingly fast math library that doesn't have legacy C++ code.

It provides the latest features, is highly customizable, has all for cheat development, DirectX/OpenGL/Vulkan support, premade support for different game engines, much more constexpr stuff than in other libraries and more...
<br>
<br>

---

**[<kbd> <br> Install <br> </kbd>][INSTALL]** 
**[<kbd> <br> Examples <br> </kbd>][EXAMPLES]** 
**[<kbd> <br> Documentation <br> </kbd>][DOCUMENTATION]** 
**[<kbd> <br> Contribute <br> </kbd>][CONTRIBUTING]** 
**[<kbd> <br> Donate <br> </kbd>][SPONSOR]** 

---

<br>

</div>


<div align = center>
 <a href="https://www.star-history.com/#orange-cpp/omath&Date">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="https://api.star-history.com/svg?repos=orange-cpp/omath&type=Date&theme=dark" />
    <source media="(prefers-color-scheme: light)" srcset="https://api.star-history.com/svg?repos=orange-cpp/omath&type=Date" />
    <img alt="Star History Chart" src="https://api.star-history.com/svg?repos=orange-cpp/omath&type=Date" />
  </picture>
 </a>
</div>

## Quick Example

```cpp
#include <omath/omath.hpp>

using namespace omath;

// 3D vector operations
Vector3<float> a{1, 2, 3};
Vector3<float> b{4, 5, 6};

auto dot = a.dot(b);              // 32.0
auto cross = a.cross(b);          // (-3, 6, -3)
auto distance = a.distance_to(b); // ~5.196
auto normalized = a.normalized(); // Unit vector

// World-to-screen projection (Source Engine example)
using namespace omath::source_engine;
Camera camera(position, angles, viewport, fov, near_plane, far_plane);

if (auto screen = camera.world_to_screen(world_position)) {
    // Draw at screen->x, screen->y
}
```

**[See more examples and tutorials][TUTORIALS]**

# Features
- **Efficiency**: Optimized for performance, ensuring quick computations using AVX2.
- **Versatility**: Includes a wide array of mathematical functions and algorithms.
- **Ease of Use**: Simplified interface for convenient integration into various projects.
- **Projectile Prediction**: Projectile prediction engine with O(N) algo complexity, that can power you projectile aim-bot.
- **3D Projection**: No need to find view-projection matrix anymore you can make your own projection pipeline.
- **Collision Detection**: Production ready code to handle collision detection by using simple interfaces.
- **No Additional Dependencies**: No additional dependencies need to use OMath except unit test execution
- **Ready for meta-programming**: Omath use templates for common types like Vectors, Matrixes etc, to handle all types!
- **Engine support**: Supports coordinate systems of **Source, Unity, Unreal, Frostbite, IWEngine and canonical OpenGL**.
- **Cross platform**: Supports Windows, MacOS and Linux.
- **Algorithms**: Has ability to scan for byte pattern with wildcards in PE files/modules, binary slices, works even with Wine apps. 
<div align = center>
 
# Gallery

<br>

[![Youtube Video](docs/images/yt_previews/img.png)](https://youtu.be/lM_NJ1yCunw?si=-Qf5yzDcWbaxAXGQ)

<br>

![APEX Preview]

<br>

![BO2 Preview]

<br>

![CS2 Preview]

<br>

![TF2 Preview]

<br>

![OpenGL Preview]

<br>
<br>

</div>

## Documentation

- **[Getting Started Guide](http://libomath.org/getting_started/)** - Installation and first steps
- **[API Overview](http://libomath.org/api_overview/)** - Complete API reference
- **[Tutorials](http://libomath.org/tutorials/)** - Step-by-step guides
- **[FAQ](http://libomath.org/faq/)** - Common questions and answers
- **[Troubleshooting](http://libomath.org/troubleshooting/)** - Solutions to common issues
- **[Best Practices](http://libomath.org/best_practices/)** - Guidelines for effective usage

## Community & Support

- **Discord**: [Join our community](https://discord.gg/eDgdaWbqwZ)
- **Telegram**: [@orangennotes](https://t.me/orangennotes)
- **Issues**: [Report bugs or request features](https://github.com/orange-cpp/omath/issues)
- **Contributing**: See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines

# Acknowledgments
-  [All contributors](https://github.com/orange-cpp/omath/graphs/contributors)

<!----------------------------------{ Images }--------------------------------->
[APEX Preview]: docs/images/showcase/apex.png
[BO2 Preview]: docs/images/showcase/cod_bo2.png
[CS2 Preview]: docs/images/showcase/cs2.jpeg
[TF2 Preview]: docs/images/showcase/tf2.jpg
[OpenGL Preview]: docs/images/showcase/opengl.png
<!----------------------------------{ Buttons }--------------------------------->
[QUICKSTART]: docs/getting_started.md
[INSTALL]: INSTALL.md
[DOCUMENTATION]: http://libomath.org
[TUTORIALS]: docs/tutorials.md
[CONTRIBUTING]: CONTRIBUTING.md
[EXAMPLES]: examples
[SPONSOR]: https://boosty.to/orangecpp/purchase/3568644?ssource=DIRECT&share=subscription_link
