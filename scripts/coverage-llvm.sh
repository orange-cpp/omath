#!/usr/bin/env bash
# scripts/coverage-llvm.sh
# LLVM coverage script that generates LCOV-style reports

set -e

SOURCE_DIR="${1:-.}"
BINARY_DIR="${2:-cmake-build/build}"
TEST_BINARY="${3:-}"
OUTPUT_DIR="${4:-${BINARY_DIR}/coverage}"

echo "[*] Source dir: ${SOURCE_DIR}"
echo "[*] Binary dir: ${BINARY_DIR}"
echo "[*] Output dir: ${OUTPUT_DIR}"

# Find llvm tools - handle versioned names (Linux) and xcrun (macOS)
find_llvm_tool() {
    local tool_name="$1"
    
    # macOS: use xcrun
    if [[ "$(uname)" == "Darwin" ]]; then
        if xcrun --find "${tool_name}" &>/dev/null; then
            echo "xcrun ${tool_name}"
            return 0
        fi
    fi
    
    # Try versioned names (Linux with LLVM 21, 20, 19, etc.)
    for version in 21 20 19 18 17 ""; do
        local versioned_name="${tool_name}${version:+-$version}"
        if command -v "${versioned_name}" &>/dev/null; then
            echo "${versioned_name}"
            return 0
        fi
    done
    
    echo ""
    return 1
}

LLVM_PROFDATA=$(find_llvm_tool "llvm-profdata")
LLVM_COV=$(find_llvm_tool "llvm-cov")

if [[ -z "${LLVM_PROFDATA}" ]] || [[ -z "${LLVM_COV}" ]]; then
    echo "Error: llvm-profdata or llvm-cov not found" >&2
    echo "On Linux, install llvm or clang package" >&2
    echo "On macOS, Xcode command line tools should provide these" >&2
    exit 1
fi

echo "[*] Using: ${LLVM_PROFDATA}"
echo "[*] Using: ${LLVM_COV}"

# Find test binary
if [[ -z "${TEST_BINARY}" ]]; then
    for path in \
        "${SOURCE_DIR}/out/Debug/unit_tests" \
        "${SOURCE_DIR}/out/Release/unit_tests" \
        "${BINARY_DIR}/unit_tests" \
        "${BINARY_DIR}/tests/unit_tests"; do
        if [[ -x "${path}" ]]; then
            TEST_BINARY="${path}"
            break
        fi
    done
fi

if [[ -z "${TEST_BINARY}" ]] || [[ ! -x "${TEST_BINARY}" ]]; then
    echo "Error: unit_tests binary not found" >&2
    echo "Searched in: out/Debug, out/Release, ${BINARY_DIR}" >&2
    exit 1
fi

echo "[*] Test binary: ${TEST_BINARY}"

# Clean previous coverage data
rm -rf "${OUTPUT_DIR}"
rm -f "${BINARY_DIR}"/*.profraw "${BINARY_DIR}"/*.profdata
mkdir -p "${OUTPUT_DIR}"

# Run tests with profiling enabled
PROFILE_FILE="${BINARY_DIR}/default_%p.profraw"
echo "[*] Running tests with LLVM_PROFILE_FILE=${PROFILE_FILE}"

export LLVM_PROFILE_FILE="${PROFILE_FILE}"
"${TEST_BINARY}" || echo "[!] Some tests failed, continuing with coverage..."

# Find all generated .profraw files
PROFRAW_FILES=$(find "${BINARY_DIR}" -name "*.profraw" -type f 2>/dev/null)
if [[ -z "${PROFRAW_FILES}" ]]; then
    # Also check current directory
    PROFRAW_FILES=$(find . -maxdepth 3 -name "*.profraw" -type f 2>/dev/null)
fi

if [[ -z "${PROFRAW_FILES}" ]]; then
    echo "Error: No .profraw files generated" >&2
    echo "Make sure the binary was built with -fprofile-instr-generate -fcoverage-mapping" >&2
    exit 1
fi

echo "[*] Found profraw files:"
echo "${PROFRAW_FILES}"

# Merge profiles
PROFDATA_FILE="${BINARY_DIR}/coverage.profdata"
echo "[*] Merging profiles into ${PROFDATA_FILE}"
${LLVM_PROFDATA} merge -sparse ${PROFRAW_FILES} -o "${PROFDATA_FILE}"

# Generate text summary
echo "[*] Coverage Summary:"
${LLVM_COV} report "${TEST_BINARY}" \
    -instr-profile="${PROFDATA_FILE}" \
    -ignore-filename-regex="tests/.*" \
    -ignore-filename-regex="googletest/.*" \
    -ignore-filename-regex="gtest/.*" \
    -ignore-filename-regex="_deps/.*" \
    -ignore-filename-regex="vcpkg_installed/.*"

# Export lcov format (for tools like codecov)
LCOV_FILE="${OUTPUT_DIR}/coverage.lcov"
echo "[*] Exporting LCOV format to ${LCOV_FILE}"
${LLVM_COV} export "${TEST_BINARY}" \
    -instr-profile="${PROFDATA_FILE}" \
    -format=lcov \
    -ignore-filename-regex="tests/.*" \
    -ignore-filename-regex="googletest/.*" \
    -ignore-filename-regex="gtest/.*" \
    -ignore-filename-regex="_deps/.*" \
    -ignore-filename-regex="vcpkg_installed/.*" \
    > "${LCOV_FILE}" || true

# Generate LCOV-style HTML report using genhtml
if command -v genhtml >/dev/null 2>&1; then
    echo "[*] Generating LCOV-style HTML report using genhtml"
    genhtml "${LCOV_FILE}" \
        --ignore-errors inconsistent,corrupt \
        --output-directory "${OUTPUT_DIR}" \
        --title "Omath Coverage Report" \
        --show-details \
        --legend \
        --demangle-cpp \
        --num-spaces 4 \
        --sort \
        --function-coverage \
        --branch-coverage
    
    echo "[*] LCOV-style HTML report generated at: ${OUTPUT_DIR}/index.html"
else
    echo "[!] genhtml not found. Installing lcov package..."
    echo "[!] On Ubuntu/Debian: sudo apt-get install lcov"
    echo "[!] On macOS: brew install lcov"
    echo "[!] Falling back to LLVM HTML report..."
    
    # Fall back to LLVM HTML report
    ${LLVM_COV} show "${TEST_BINARY}" \
        -instr-profile="${PROFDATA_FILE}" \
        -format=html \
        -output-dir="${OUTPUT_DIR}" \
        -show-line-counts-or-regions \
        -show-instantiations=false \
        -ignore-filename-regex="tests/.*" \
        -ignore-filename-regex="googletest/.*" \
        -ignore-filename-regex="gtest/.*" \
        -ignore-filename-regex="_deps/.*" \
        -ignore-filename-regex="vcpkg_installed/.*"
fi

echo "[*] Coverage report generated at: ${OUTPUT_DIR}/index.html"
echo "[*] LCOV file at: ${LCOV_FILE}"
