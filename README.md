<div align = center>

![banner](https://i.imgur.com/sjtpKi8.png)

![GitHub License](https://img.shields.io/github/license/orange-cpp/omath)
![GitHub contributors](https://img.shields.io/github/contributors/orange-cpp/omath)
![GitHub top language](https://img.shields.io/github/languages/top/orange-cpp/omath)
[![CodeFactor](https://www.codefactor.io/repository/github/orange-cpp/omath/badge)](https://www.codefactor.io/repository/github/orange-cpp/omath)
![GitHub forks](https://img.shields.io/github/forks/orange-cpp/omath)
</div>

Oranges's Math Library (omath) is a comprehensive, open-source library aimed at providing efficient, reliable, and versatile mathematical functions and algorithms. Developed primarily in C++, this library is designed to cater to a wide range of mathematical operations essential in scientific computing, engineering, and academic research.

## Features
- **Efficiency**: Optimized for performance, ensuring quick computations.
- **Versatility**: Includes a wide array of mathematical functions and algorithms.
- **Ease of Use**: Simplified interface for convenient integration into various projects.
- **Projectile Prediction**: Projectile prediction engine with O(N) algo complexity, that can power you projectile aim-bot.
- **3D Projection**: No need to find view-projection matrix anymore you can make your own projection pipeline.
- **Collision Detection**: Production ready code to handle collision detection by using simple interfaces.
- **No Additional Dependencies**: No additional dependencies need to use OMath except unit test execution

## Getting Started
### Prerequisites
- C++ Compiler
- CMake (for building the project)

### Installation
1. Clone the repository:
   ```
   git clone https://github.com/orange-cpp/omath.git
   ```
2. Navigate to the project directory:
   ```
   cd omath
   ```
3. Build the project using CMake:
   ```
   cmake --preset windows-release -S .
   cmake --build cmake-build/build/windows-release --target server -j 6
   ```
   Use **\<platform\>-\<build configuration\>** preset to build siutable version for yourself. Like **windows-release** or **linux-release**.
## Usage
Simple world to screen function
```c++
TEST(UnitTestProjection, IsPointOnScreen)
{
    const omath::projection::Camera camera({0.f, 0.f, 0.f}, {0, 0.f, 0.f} , {1920.f, 1080.f}, 110.f, 0.1f, 500.f);

    const auto proj = camera.WorldToScreen({100, 0, 15});
    EXPECT_TRUE(proj.has_value());
}
```

<details>
  <summary>OMATH for making cheats</summary>

With `omath/projection` module you can achieve simple ESP hack for powered by Source/Unreal/Unity engine games, like [Apex Legends](https://store.steampowered.com/app/1172470/Apex_Legends/).

![banner](https://i.imgur.com/lcJrfcZ.png)
![Watch Video](https://youtu.be/lM_NJ1yCunw?si=5E87OrQMeypxSJ3E)

</details>

## Contributing
Contributions to `omath` are welcome! Please read `CONTRIBUTING.md` for details on our code of conduct and the process for submitting pull requests.

## License
This project is licensed under the MIT - see the `LICENSE` file for details.

## Acknowledgments
- Orange | [Telegram](https://t.me/orange_cpp)
