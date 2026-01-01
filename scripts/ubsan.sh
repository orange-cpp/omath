#!/bin/bash

# =============================================================================
# Local Reproduction of "Linux (UBSan)" GitHub Action
# =============================================================================

# Stop on error, undefined variables, or pipe failures
set -euo pipefail

# --- 1. Environment Setup ---

# Determine VCPKG_ROOT
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

PRESET="linux-release-vcpkg"
BUILD_DIR="cmake-build/build/${PRESET}"
UNIT_TESTS_BIN="./out/RelWithDebInfo/unit_tests"
TRIPLET="x64-linux-ubsan"

echo "----------------------------------------------------"
echo "Starting Configuration (UBSan)..."
echo "----------------------------------------------------"

# Set Clang compiler if not already set
if [[ "${CC:-}" != *"clang"* ]]; then
    export CC=/usr/bin/clang
fi
if [[ "${CXX:-}" != *"clang"* ]]; then
    export CXX=/usr/bin/clang++
fi

# --- 2. Configure (CMake) ---
cmake --preset "${PRESET}" \
  -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DOMATH_UBSAN=ON \
  -DOMATH_BUILD_TESTS=ON \
  -DVCPKG_MANIFEST_FEATURES="tests" \
  -DVCPKG_OVERLAY_TRIPLETS="$(pwd)/cmake/triplets" \
  -DVCPKG_TARGET_TRIPLET="${TRIPLET}"

echo "----------------------------------------------------"
echo "Building Targets..."
echo "----------------------------------------------------"

# --- 3. Build ---
cmake --build "${BUILD_DIR}" --target unit_tests

echo "----------------------------------------------------"
echo "Running Unit Tests (UBSan)..."
echo "----------------------------------------------------"

# --- 4. Run Tests ---
export UBSAN_OPTIONS=print_stacktrace=1
"${UNIT_TESTS_BIN}"
