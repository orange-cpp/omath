# Universal Math Library (UML)

## Overview
The Universal Math Library (UML) is a comprehensive, open-source library aimed at providing efficient, reliable, and versatile mathematical functions and algorithms. Developed primarily in C++, this library is designed to cater to a wide range of mathematical operations essential in scientific computing, engineering, and academic research.

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
   git clone https://github.com/VladislavAlpatov/uml.git
   ```
2. Navigate to the project directory:
   ```
   cd uml
   ```
3. Build the project using CMake:
   ```
   cmake --preset x64-release -S .
   cmake --build cmake-build/build/x64-release --target server -j 6
   ```

## Usage
Simple world to screen function
```c++
std::optional<uml::Vector3> WorldToScreen(uml::Vector3 worldPosition, float width, float height)
    {
        auto projected = (GetViewProjectionMatrix() * worldPosition).transpose();

        projected /= projected.at(0, 3);

        const auto out = projected * uml::matrix::to_screen_matrix(width,
                                                                   height);

        if (out.at(0,2) <= 0.f)
            return std::nullopt;
        auto final = uml::Vector3(out.at(0,0),
                                  out.at(0, 1),
                                  out.at(0,2));
        return {final};
    }
```
## Contributing
Contributions to UML are welcome! Please read `CONTRIBUTING.md` for details on our code of conduct and the process for submitting pull requests.

## License
This project is licensed under the GPL V3 - see the `LICENSE` file for details.

## Acknowledgments
- Vladislav Alpatov | [Telegram](https://t.me/nullifiedvlad)
