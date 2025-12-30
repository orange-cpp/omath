#!/bin/bash

# =============================================================================
# Local Reproduction of "Linux Coverage" GitHub Action
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

# Check for required tools
if ! command -v lcov &> /dev/null; then
    echo "Error: lcov is not installed. Please install it (e.g., sudo apt install lcov)."
    exit 1
fi

# Set the build directory matching the YAML's preset expectation
PRESET="linux-release-vcpkg"
BUILD_DIR="cmake-build/build/${PRESET}"
COVERAGE_DIR="${BUILD_DIR}/coverage"
UNIT_TESTS_BIN="./out/Release/unit_tests"

echo "----------------------------------------------------"
echo "Starting Configuration (Coverage Build)..."
echo "----------------------------------------------------"

# --- 2. Configure (CMake) ---
cmake --preset "${PRESET}" \
  -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
  -DVCPKG_INSTALL_OPTIONS="--allow-unsupported" \
  -DOMATH_BUILD_TESTS=ON \
  -DOMATH_BUILD_BENCHMARK=OFF \
  -DOMATH_ENABLE_COVERAGE=ON \
  -DVCPKG_MANIFEST_FEATURES="imgui;avx2;tests"

echo "----------------------------------------------------"
echo "Building Targets..."
echo "----------------------------------------------------"

# --- 3. Build ---
cmake --build "${BUILD_DIR}" --target unit_tests omath

echo "----------------------------------------------------"
echo "Running Unit Tests..."
echo "----------------------------------------------------"

# --- 4. Run Tests ---
# We run the tests to generate the profile data
"${UNIT_TESTS_BIN}"

echo "----------------------------------------------------"
echo "Generating Coverage Report..."
echo "----------------------------------------------------"

# --- 5. Run Coverage Script ---
# Ensure the script is executable
chmod +x scripts/coverage-llvm.sh

./scripts/coverage-llvm.sh \
  "$(pwd)" \
  "${BUILD_DIR}" \
  "${UNIT_TESTS_BIN}" \
  "${COVERAGE_DIR}"

echo "----------------------------------------------------"
echo "Coverage report generated at: ${COVERAGE_DIR}/index.html"
echo "----------------------------------------------------"
