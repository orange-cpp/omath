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
   cmake --preset x64-release -S .
   cmake --build cmake-build/build/x64-release --target server -j 6
   ```

## Usage
Simple world to screen function
```c++
std::optional<omath::Vector3> WorldToScreen(omath::Vector3 worldPosition, float width, float height)
    {
        auto projected = (GetViewProjectionMatrix() * worldPosition).transpose();

        projected /= projected.at(0, 3);

        const auto out = projected * omath::matrix::to_screen_matrix(width,
                                                                     height);

        if (out.at(0, 2) <= 0.f)
            return std::nullopt;
        auto final = omath::Vector3(out.at(0, 0),
                                    out.at(0, 1),
                                    out.at(0, 3));
        return {final};
    }
```
## Contributing
Contributions to `omath` are welcome! Please read `CONTRIBUTING.md` for details on our code of conduct and the process for submitting pull requests.

## License
This project is licensed under the GPL V3 - see the `LICENSE` file for details.

## Acknowledgments
- Orange | [Telegram](https://t.me/orange_cpp)
