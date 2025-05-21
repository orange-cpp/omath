# 📥Installation Guide

## Using vcpkg
**Note**: Support vcpkg for package management
1. Install [vcpkg](https://github.com/microsoft/vcpkg)
2. Run the following command to install the orange-math package:
```
vcpkg install orange-math
```
CMakeLists.txt
```cmake
find_package(omath CONFIG REQUIRED)
target_link_libraries(main PRIVATE omath::omath)
```
For detailed commands on installing different versions and more information, please refer to Microsoft's [official instructions](https://learn.microsoft.com/en-us/vcpkg/get_started/overview).

## Build from source using CMake
1. **Preparation**

   Install needed tools: cmake, clang, git, msvc (windows only).

    1. **Linux:**
        ```bash
        sudo pacman -Sy cmake ninja clang git
        ```
    2. **MacOS:**
        ```bash
        brew install llvm git cmake ninja
        ```
    3. **Windows:**

       Install Visual Studio from [here](https://visualstudio.microsoft.com/downloads/) and Git from [here](https://git-scm.com/downloads).

       Use x64 Native Tools shell to execute needed commands down below.
2. **Clone the repository:**
   ```bash
   git clone https://github.com/orange-cpp/omath.git
   ```
3. **Navigate to the project directory:**
   ```bash
   cd omath
   ```
4. **Build the project using CMake:**
   ```bash
   cmake --preset windows-release -S .
   cmake --build cmake-build/build/windows-release --target omath -j 6
   ```
   Use **\<platform\>-\<build configuration\>** preset to build siutable version for yourself. Like **windows-release** or **linux-release**.

   | Platform Name | Build Config  |
       |---------------|---------------|
   | windows       | release/debug |
   | linux         | release/debug |
   | darwin        | release/debug |