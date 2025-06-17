<div align = center>

![banner](raw.githubusercontent.com/orange-cpp/omath/refs/heads/main/.github/images/banner.png)

![GitHub License](https://img.shields.io/github/license/orange-cpp/omath)
![GitHub contributors](https://img.shields.io/github/contributors/orange-cpp/omath)
![GitHub top language](https://img.shields.io/github/languages/top/orange-cpp/omath)
[![CodeFactor](https://www.codefactor.io/repository/github/orange-cpp/omath/badge)](https://www.codefactor.io/repository/github/orange-cpp/omath)
![GitHub Actions Workflow Status](https://img.shields.io/github/actions/workflow/status/orange-cpp/omath/cmake-multi-platform.yml)
![GitHub forks](https://img.shields.io/github/forks/orange-cpp/omath)
</div>

Oranges's Math Library (omath) is a comprehensive, open-source library aimed at providing efficient, reliable, and versatile mathematical functions and algorithms. Developed primarily in C++, this library is designed to cater to a wide range of mathematical operations essential in scientific computing, engineering, and academic research.

## 👁‍🗨 Features
- **Efficiency**: Optimized for performance, ensuring quick computations using AVX2.
- **Versatility**: Includes a wide array of mathematical functions and algorithms.
- **Ease of Use**: Simplified interface for convenient integration into various projects.
- **Projectile Prediction**: Projectile prediction engine with O(N) algo complexity, that can power you projectile aim-bot.
- **3D Projection**: No need to find view-projection matrix anymore you can make your own projection pipeline.
- **Collision Detection**: Production ready code to handle collision detection by using simple interfaces.
- **No Additional Dependencies**: No additional dependencies need to use OMath except unit test execution
- **Ready for meta-programming**: Omath use templates for common types like Vectors, Matrixes etc, to handle all types!

## Supported Render Pipelines
| ENGINE   | SUPPORT |
|----------|---------|
| Source   | ✅YES    |
| Unity    | ✅YES    |
| IWEngine | ✅YES    |
| Unreal   | ❌NO     |

## Supported Operating Systems

| OS             | SUPPORT |
|----------------|---------|
| Windows 10/11  | ✅YES    |
| Linux          | ✅YES    |
| Darwin (MacOS) | ✅YES    |

## ⏬ Installation
Please read our [installation guide](https://github.com/orange-cpp/omath/blob/main/INSTALL.md). If this link doesn't work check out INSTALL.md file.

## ❔ Usage
Simple world to screen function
```c++
TEST(UnitTestProjection, IsPointOnScreen)
{
    const omath::projection::Camera camera({0.f, 0.f, 0.f}, {0, 0.f, 0.f} , {1920.f, 1080.f}, 110.f, 0.1f, 500.f);

    const auto proj = camera.WorldToScreen({100, 0, 15});
    EXPECT_TRUE(proj.has_value());
}
```
## Showcase
<details>
  <summary>OMATH for making cheats (click to open)</summary>

With `omath/projection` module you can achieve simple ESP hack for powered by Source/Unreal/Unity engine games, like [Apex Legends](https://store.steampowered.com/app/1172470/Apex_Legends/).

![banner](https://i.imgur.com/lcJrfcZ.png)
Or for InfinityWard Engine based games. Like Call of Duty Black Ops 2!
![banner](https://i.imgur.com/F8dmdoo.png)
Or create simple trigger bot with embeded traceline from omath::collision::LineTrace
![banner](https://i.imgur.com/fxMjRKo.jpeg)
Or even advanced projectile aimbot
[Watch Video](https://youtu.be/lM_NJ1yCunw?si=5E87OrQMeypxSJ3E)
</details>

## 🫵🏻 Contributing
Contributions to `omath` are welcome! Please read `CONTRIBUTING.md` for details on our code of conduct and the process for submitting pull requests.

## 📜 License
This project is licensed under the MIT - see the `LICENSE` file for details.

## 💘 Acknowledgments
-  [All contributors](https://github.com/orange-cpp/omath/graphs/contributors)
