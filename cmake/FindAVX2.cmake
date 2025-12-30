include(CheckCXXSourceCompiles) 
include(CheckCXXSourceRuns)
include(FindPackageHandleStandardArgs)

set(AVX2_TEST_SOURCE [[
#include <immintrin.h>
#include <stdint.h>

int main(int argc, char** argv) {
    __m256i a = _mm256_set1_epi32(argc); 
    __m256i b = _mm256_set1_epi32(2);
    __m256i c = _mm256_add_epi32(a, b);
    alignas(32) int32_t result[8];
    _mm256_store_si256((__m256i*)result, c);
    return (result[0] == (argc + 2)) ? 0 : 1;
}
]])

if(TARGET SIMD::AVX2)
    return()
endif()

if(MSVC)
    set(_AVX2_CANDIDATES "/arch:AVX2")
else()
    set(_AVX2_CANDIDATES
        "-mfma -mavx2"
        "-msimd128 -mavx2"
        "-mavx2"
        "-march=core-avx2"
        " "
    )
endif()

set(AVX2_WORKING_FLAG "")
foreach(FLAG IN LISTS _AVX2_CANDIDATES)
    string(MAKE_C_IDENTIFIER "AVX2_TEST_FLAG_${FLAG}" _CHECK_VAR)
    set(CMAKE_REQUIRED_FLAGS "${FLAG}")
    if(ANDROID OR IOS OR EMSCRIPTEN)
        check_cxx_source_compiles("${AVX2_TEST_SOURCE}" ${_CHECK_VAR})
    else()
        check_cxx_source_runs("${AVX2_TEST_SOURCE}" ${_CHECK_VAR})
    endif()
    if(${_CHECK_VAR})
        set(AVX2_WORKING_FLAG "${FLAG}")
        break()
    endif()
endforeach()

find_package_handle_standard_args(AVX2
    REQUIRED_VARS AVX2_WORKING_FLAG
    REASON_FAILURE_MESSAGE "Your CPU or compiler does not support AVX2 instruction sets."
)

if(AVX2_FOUND)
    add_library(SIMD::AVX2 INTERFACE IMPORTED)
    string(STRIP "${AVX2_WORKING_FLAG}" _CLEAN_FLAG)
    if(_CLEAN_FLAG)
        target_compile_options(SIMD::AVX2 INTERFACE ${_CLEAN_FLAG})
    endif()
    target_compile_definitions(SIMD::AVX2 INTERFACE OMATH_USE_AVX2)
    set(AVX2_FLAGS "${AVX2_WORKING_FLAG}" CACHE STRING "Calculated AVX2 compiler flags")
    mark_as_advanced(AVX2_FLAGS)
endif()
