<div align = center>

![banner](.github/images/logos/omath_logo_macro.png)

![Static Badge](https://img.shields.io/badge/license-libomath-orange)
![GitHub contributors](https://img.shields.io/github/contributors/orange-cpp/omath)
![GitHub top language](https://img.shields.io/github/languages/top/orange-cpp/omath)
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
**[<kbd> <br> Contribute <br> </kbd>][CONTRIBUTING]**

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

## 👁‍🗨 Features
- **Efficiency**: Optimized for performance, ensuring quick computations using AVX2.
- **Versatility**: Includes a wide array of mathematical functions and algorithms.
- **Ease of Use**: Simplified interface for convenient integration into various projects.
- **Projectile Prediction**: Projectile prediction engine with O(N) algo complexity, that can power you projectile aim-bot.
- **3D Projection**: No need to find view-projection matrix anymore you can make your own projection pipeline.
- **Collision Detection**: Production ready code to handle collision detection by using simple interfaces.
- **No Additional Dependencies**: No additional dependencies need to use OMath except unit test execution
- **Ready for meta-programming**: Omath use templates for common types like Vectors, Matrixes etc, to handle all types!

# Gallery

<br>

[![Youtube Video](.github/images/yt_previews/img.png)](https://youtu.be/lM_NJ1yCunw?si=-Qf5yzDcWbaxAXGQ)

<br>

![APEX Preview]

<br>

![BO2 Preview]

<br>

![CS2 Preview]

<br>
<br>

</div>


## Supported Render Pipelines
| ENGINE   | SUPPORT |
|----------|---------|
| Source   | ✅YES    |
| Unity    | ✅YES    |
| IWEngine | ✅YES    |
| OpenGL   | ✅YES    |
| Unreal   | ✅YES    |

## Supported Operating Systems

| OS             | SUPPORT |
|----------------|---------|
| Windows 10/11  | ✅YES    |
| Linux          | ✅YES    |
| Darwin (MacOS) | ✅YES    |

## ❔ Usage
ESP example
```c++
omath::source_engine::Camera cam{localPlayer.GetCameraOrigin(),
                                 localPlayer.GetAimPunch(),
                                 {1920.f, 1080.f},
                                 localPlayer.GetFieldOfView(),
                                 0.01.f, 30000.f};

for (auto ent: apex_sdk::EntityList::GetAllEntities())
{
    const auto bottom = cam.world_to_screen(ent.GetOrigin());
    const auto top = cam.world_to_screen(ent.GetBonePosition(8) + omath::Vector3<float>{0, 0, 10});

    const auto ent_health = ent.GetHealth();

    if (!top || !bottom || ent_health <= 0)
        continue;
    // esp rendering...
}
```

## 💘 Acknowledgments
-  [All contributors](https://github.com/orange-cpp/omath/graphs/contributors)

<!----------------------------------{ Images }--------------------------------->
[APEX Preview]: .github/images/showcase/apex.png
[BO2 Preview]: .github/images/showcase/cod_bo2.png
[CS2 Preview]: .github/images/showcase/cs2.jpeg

<!----------------------------------{ Buttons }--------------------------------->
[INSTALL]: INSTALL.md
[CONTRIBUTING]: CONTRIBUTING.md
[EXAMPLES]: examples
