# 📥Installation Guide

## <img width="28px" src="https://vcpkg.io/assets/mark/mark.svg" /> Using vcpkg (recomended)
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

## <img width="28px" src="https://xmake.io/assets/img/logo.svg" /> Using xrepo
**Note**: Support xrepo for package management
1. Install [xmake](https://xmake.io/)
2. Run the following command to install the omath package:
```
xrepo install omath
```
xmake.lua
```xmake
add_requires("omath")
target("...")
    add_packages("omath")
```

## <img width="28px" src="https://github.githubassets.com/favicons/favicon.svg" /> Using prebuilt binaries (GitHub Releases)

**Note**: This is the fastest option if you don’t want to build from source.

1. **Go to the Releases page**
   - Open the project’s GitHub **Releases** page and choose the latest version.

2. **Download the correct asset for your platform**
   - Pick the archive that matches your OS and architecture (for example: Windows x64 / Linux x64 / macOS arm64).

3. **Extract the archive**
   - You should end up with something like:
     - `include/` (headers)
     - `lib/` or `bin/` (library files / DLLs)
     - sometimes `cmake/` (CMake package config)

4. **Use it in your project**

   ### Option A: CMake package (recommended if the release includes CMake config files)
   If the extracted folder contains something like `lib/cmake/omath` or `cmake/omath`, you can point CMake to it:

   ```cmake
   # Example: set this to the extracted prebuilt folder
   list(APPEND CMAKE_PREFIX_PATH "path/to/omath-prebuilt")

   find_package(omath CONFIG REQUIRED)
   target_link_libraries(main PRIVATE omath::omath)
   ```
   ### Option B: Manual include + link (works with any layout)
   If there’s no CMake package config, link it manually:
   ```cmake
   target_include_directories(main PRIVATE "path/to/omath-prebuilt/include")

    # Choose ONE depending on what you downloaded:
    # - Static library: .lib / .a
    # - Shared library: .dll + .lib import (Windows), .so (Linux), .dylib (macOS)
    
    target_link_directories(main PRIVATE "path/to/omath-prebuilt/lib")
    target_link_libraries(main PRIVATE omath)  # or the actual library filename
    ```
## <img width="28px" src="https://upload.wikimedia.org/wikipedia/commons/e/ef/CMake_logo.svg?" /> Build from source using CMake 
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
   Use **\<platform\>-\<build configuration\>** preset to build suitable version for yourself. Like **windows-release** or **linux-release**.

    | Platform Name | Build Config  |
    |---------------|---------------|
    | windows       | release/debug |
    | linux         | release/debug |
    | darwin        | release/debug |
