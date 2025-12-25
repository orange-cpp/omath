#!/bin/bash

# =============================================================================
# Local Reproduction of "Valgrind Analysis" GitHub Action
# =============================================================================

# Stop on error, undefined variables, or pipe failures
set -euo pipefail

# --- 1. Environment Setup ---

# Determine VCPKG_ROOT
# If VCPKG_ROOT is not set in your shell, we check if a local folder exists.
if [[ -z "${VCPKG_ROOT:-}" ]]; then
    if [[ -d "./vcpkg" ]]; then
        export VCPKG_ROOT="$(pwd)/vcpkg"
        echo "Found local vcpkg at: $VCPKG_ROOT"
        
        # Bootstrap vcpkg if the executable doesn't exist
        if [[ ! -f "$VCPKG_ROOT/vcpkg" ]]; then
            echo "Bootstrapping vcpkg..."
            "$VCPKG_ROOT/bootstrap-vcpkg.sh"
        fi
    else
        echo "Error: VCPKG_ROOT is not set and ./vcpkg directory not found."
        echo "Please install vcpkg or set the VCPKG_ROOT environment variable."
        exit 1
    fi
else
    echo "Using existing VCPKG_ROOT: $VCPKG_ROOT"
fi

# Set the build directory matching the YAML's preset expectation
# Assuming the preset writes to: cmake-build/build/linux-release-vcpkg
BUILD_DIR="cmake-build/build/linux-release-vcpkg"

# Check if Valgrind is installed
if ! command -v valgrind &> /dev/null; then
    echo "Error: valgrind is not installed. Please install it (e.g., sudo apt install valgrind)."
    exit 1
fi

echo "----------------------------------------------------"
echo "Starting Configuration (Debug Build with Valgrind)..."
echo "----------------------------------------------------"

# --- 2. Configure (CMake) ---
# We force CMAKE_BUILD_TYPE=Debug even though the preset says 'release'
# to ensure Valgrind has access to debug symbols (line numbers).

cmake --preset linux-release-vcpkg \
    -DCMAKE_BUILD_TYPE=Debug \
    -DOMATH_BUILD_EXAMPLES=OFF \
    -DOMATH_BUILD_TESTS=ON \
    -DOMATH_BUILD_BENCHMARK=ON \
    -DOMATH_ENABLE_VALGRIND=ON \
    -DVCPKG_MANIFEST_FEATURES="imgui;avx2;tests;benchmark"

echo "----------------------------------------------------"
echo "Building Targets..."
echo "----------------------------------------------------"

# --- 3. Build ---
# Using the specific build directory defined by the preset structure
cmake --build "$BUILD_DIR"

echo "----------------------------------------------------"
echo "Running Valgrind Analysis..."
echo "----------------------------------------------------"

# --- 4. Run Valgrind ---
# Runs the specific custom target defined in your CMakeLists.txt
cmake --build "$BUILD_DIR" --target valgrind_all

echo "----------------------------------------------------"
echo "Valgrind Analysis Complete."
echo "----------------------------------------------------"
