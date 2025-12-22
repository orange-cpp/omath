#!/usr/bin/env bash
set -euo pipefail

usage() {
    cat <<EOF
Usage: $0 [build-dir]

Runs a Clang-instrumented coverage build and produces HTML via llvm-profdata/llvm-cov.

Default build dir: build/clang-coverage

Steps performed:
  - configure CMake with Clang and coverage flags
  - build the unit_tests target
  - run tests with LLVM_PROFILE_FILE to emit .profraw
  - merge profiles with llvm-profdata
  - render HTML with llvm-cov
EOF
}

BUILD_DIR=${1:-build/clang-coverage}
NPROC=${NPROC:-$(nproc)}
NO_AVX=${NO_AVX:-0}

for cmd in clang clang++ cmake llvm-profdata llvm-cov; do
    if ! command -v "$cmd" >/dev/null 2>&1; then
        echo "Required tool '$cmd' not found on PATH" 1>&2
        exit 1
    fi
done

VCPKG_FLAG=""
if [ -n "${VCPKG_ROOT:-}" ] && [ -f "$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" ]; then
    echo "[*] Using vcpkg toolchain from: $VCPKG_ROOT"
    VCPKG_FLAG="-DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
fi

VCPKG_FEATURES="${VCPKG_MANIFEST_FEATURES:-tests}"
VCPKG_OPTS="${VCPKG_INSTALL_OPTIONS:---allow-unsupported}"

echo "[*] Configuring Clang coverage build in: ${BUILD_DIR}"
CC=$(command -v clang)
CXX=$(command -v clang++)
cmake_args=( 
    -S . -B "${BUILD_DIR}" 
    -DCMAKE_BUILD_TYPE=Debug 
    -DOMATH_BUILD_TESTS=ON
    -DOMATH_ENABLE_COVERAGE=ON
    -DCMAKE_C_COMPILER="${CC}" 
    -DCMAKE_CXX_COMPILER="${CXX}"
)

if [ -n "${VCPKG_FLAG}" ]; then
    cmake_args+=( "${VCPKG_FLAG}" )
    cmake_args+=( "-DVCPKG_MANIFEST_FEATURES=${VCPKG_FEATURES}" )
    cmake_args+=( "-DVCPKG_INSTALL_OPTIONS=${VCPKG_OPTS}" )
fi

if [ "${NO_AVX}" = "1" ]; then
    echo "[*] Disabling AVX2 for this build (NO_AVX=1)"
    cmake_args+=( -DOMATH_USE_AVX2=OFF )
fi

cmake "${cmake_args[@]}"

echo "[*] Building unit_tests target"
cmake --build "${BUILD_DIR}" --target unit_tests -j"${NPROC}"

echo "[*] Running tests (profiles will be written to ${BUILD_DIR})"
export LLVM_PROFILE_FILE="${BUILD_DIR}/unit_tests.%p.profraw"

# Try ctest first, fall back to running the binary directly
if command -v ctest >/dev/null 2>&1; then
    ctest --test-dir "${BUILD_DIR}" --output-on-failure || true
fi

if [ -x "out/Debug/unit_tests" ]; then
    echo "[*] Running test binary directly: out/Debug/unit_tests"
    ./out/Debug/unit_tests || true
fi

echo "[*] Locating .profraw files (searching build dir, out, and repo root)"
# Look in several likely locations to be robust across different CMake output layouts
mapfile -t profraws < <(find "${BUILD_DIR}" out . -type f -name '*.profraw' -print 2>/dev/null | sort -u)
if [ ${#profraws[@]} -eq 0 ]; then
    echo "No .profraw files were generated. Ensure the tests were run under the Clang-instrumented build." 1>&2
    echo "Searched locations: ${BUILD_DIR}, out, and current repo root."
    exit 1
fi

echo "[*] Merging ${#profraws[@]} profraw(s) into coverage.profdata"
# Use absolute paths when merging to avoid cwd-related surprises
abs_profraws=()
for p in "${profraws[@]}"; do
    if command -v realpath >/dev/null 2>&1; then
        abs_profraws+=( "$(realpath "$p")" )
    else
        abs_profraws+=( "$(cd "$(dirname "$p")" && pwd)/$(basename "$p")" )
    fi
done
llvm-profdata merge -sparse "${abs_profraws[@]}" -o "${BUILD_DIR}/coverage.profdata"

echo "[*] Locating test binary for llvm-cov"
test_bin=$(find out -type f -executable -name unit_tests -print -quit || true)
if [ -z "${test_bin}" ]; then
    echo "unit_tests binary not found (expected out/Debug/unit_tests)." 1>&2
    exit 1
fi
# Normalize to absolute path so llvm-cov can be run from the build directory
if command -v realpath >/dev/null 2>&1; then
    test_bin_abs=$(realpath "${test_bin}")
else
    test_bin_abs="$(cd "$(dirname "${test_bin}")" && pwd)/$(basename "${test_bin}")"
fi

echo "[*] Generating HTML with llvm-cov"
# Some builds rewrite source paths (via -ffile-prefix-map) to relative
# locations like ../include/..., so ensure the build dir has symlinks
# back to the source include/source trees so llvm-cov can open them.
src_root=$(pwd)
created_links=()
if [ ! -e "${BUILD_DIR}/include" ]; then
    ln -s "${src_root}/include" "${BUILD_DIR}/include"
    created_links+=("${BUILD_DIR}/include")
fi
if [ ! -e "${BUILD_DIR}/source" ]; then
    ln -s "${src_root}/source" "${BUILD_DIR}/source"
    created_links+=("${BUILD_DIR}/source")
fi

# Also create symlinks at the build-dir parent so paths like ../include/... from
# the build subdirectory resolve correctly (some profiles record SFs with
# an extra '..' component). For BUILD_DIR=build/clang-coverage this creates
# build/include -> ../include (repo include) and build/source similarly.
build_parent=$(dirname "${BUILD_DIR}")
if [ ! -e "${build_parent}/include" ]; then
    ln -s "${src_root}/include" "${build_parent}/include"
    created_links+=("${build_parent}/include")
fi
if [ ! -e "${build_parent}/source" ]; then
    ln -s "${src_root}/source" "${build_parent}/source"
    created_links+=("${build_parent}/source")
fi

echo "[*] Running llvm-cov from build directory to resolve relative source paths"
pushd "${BUILD_DIR}" >/dev/null
profdata="$(pwd)/coverage.profdata"
# Also make a copy inside the coverage output directory so other tools/scripts can find it there
mkdir -p coverage
cp -f "${profdata}" coverage/coverage.profdata || true
llvm-cov show "${test_bin_abs}" -instr-profile="${profdata}" -format=html -output-dir "coverage"

echo "[*] Exporting LCOV-format coverage"
# Export lcov-style coverage so downstream tools that expect .info/.lcov can consume it
llvm-cov export "${test_bin_abs}" -instr-profile="${profdata}" -format=lcov > coverage/coverage.lcov || true
# Provide a .info alias for tools that look for that extension
cp -f coverage/coverage.lcov coverage/coverage.info || true

echo "[*] Generating LCOV HTML with genhtml (will rewrite SF paths if needed)"
# genhtml expects absolute source-file (SF:) paths or reachable relative paths. Some builds
# record SF entries like "SF:../include/...". Rewrite common repo-relative prefixes to
# absolute paths inside the coverage.lcov to ensure genhtml can open sources.
LCOV_FILE=coverage/coverage.lcov
FIXED_LCOV=coverage/coverage.fixed.lcov
REPO_ROOT="${src_root}"
if [ -f "${LCOV_FILE}" ]; then
    sed -e "s|SF:../include/|SF:${REPO_ROOT}/include/|g" \
        -e "s|SF:../source/|SF:${REPO_ROOT}/source/|g" \
        -e "s|SF:include/|SF:${REPO_ROOT}/include/|g" \
        -e "s|SF:source/|SF:${REPO_ROOT}/source/|g" \
        "${LCOV_FILE}" > "${FIXED_LCOV}" || cp -f "${LCOV_FILE}" "${FIXED_LCOV}"
    if command -v genhtml >/dev/null 2>&1; then
        genhtml "${FIXED_LCOV}" -o coverage/lcov-html || true
        echo "[*] LCOV HTML available at: ${BUILD_DIR}/coverage/lcov-html/index.html"
    else
        echo "[*] genhtml not found on PATH; skipping LCOV HTML generation (coverage.lcov created)"
    fi
else
    echo "[*] LCOV file not found: ${LCOV_FILE}; skipping genhtml step"
fi
popd >/dev/null

# Cleanup temporary symlinks we created
for p in "${created_links[@]}"; do
    if [ -L "${p}" ]; then
        rm "${p}"
    fi
done

echo "[*] Coverage HTML available at: ${BUILD_DIR}/coverage/index.html"
