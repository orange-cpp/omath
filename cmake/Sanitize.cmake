# ASAN
option(OMATH_ASAN "Instrument executable with address sanitizer" OFF)
if(OMATH_ASAN)
    message(STATUS "omath: Address sanitizer enabled.")
    set(CMAKE_CXX_FLAGS_DEBUG
        "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address,leak -fno-sanitize-recover=address,leak"
    )
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO
        "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -fno-omit-frame-pointer -fsanitize=address,leak -fno-sanitize-recover=address,leak"
    )
    set(CMAKE_CXX_FLAGS_RELEASE
        "${CMAKE_CXX_FLAGS_RELEASE} -fno-omit-frame-pointer -fsanitize=address,leak -fno-sanitize-recover=address,leak"
    )
else()
    message(STATUS "omath: Address sanitizer disabled.")
endif()

# MSAN
option(OMATH_MSAN "Instrument executable with memory sanitizer" OFF)
if(OMATH_MSAN)
    message(STATUS "omath: Memory sanitizer enabled.")
    set(CMAKE_CXX_FLAGS_DEBUG
        "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=memory -fno-sanitize-recover=memory"
    )
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO
        "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -fno-omit-frame-pointer -fsanitize=memory -fno-sanitize-recover=memory"
    )
    set(CMAKE_CXX_FLAGS_RELEASE
        "${CMAKE_CXX_FLAGS_RELEASE} -fno-omit-frame-pointer -fsanitize=memory -fno-sanitize-recover=memory"
    )
else()
    message(STATUS "omath: Memory sanitizer disabled.")
endif()

# TSAN
option(OMATH_TSAN "Instrument executable with thread sanitizer" OFF)
if(OMATH_TSAN)
    message(STATUS "omath: Thread sanitizer enabled.")
    set(CMAKE_CXX_FLAGS_DEBUG
        "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=thread -fno-sanitize-recover=thread"
    )
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO
        "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -fno-omit-frame-pointer -fsanitize=thread -fno-sanitize-recover=thread"
    )
    set(CMAKE_CXX_FLAGS_RELEASE
        "${CMAKE_CXX_FLAGS_RELEASE} -fno-omit-frame-pointer -fsanitize=thread -fno-sanitize-recover=thread"
    )
else()
    message(STATUS "omath: Thread sanitizer disabled.")
endif()

# UBSAN
option(OMATH_UBSAN "Instrument executable with undefined behavior sanitizer"
       OFF)
if(OMATH_UBSAN)
    message(STATUS "omath: Undefined behavior sanitizer enabled.")
    set(CMAKE_CXX_FLAGS_DEBUG
        "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=undefined,float-divide-by-zero,alignment,float-cast-overflow -fno-sanitize-recover=undefined,float-divide-by-zero,alignment,float-cast-overflow"
    )
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO
        "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -fno-omit-frame-pointer -fsanitize=undefined,float-divide-by-zero,alignment,float-cast-overflow -fno-sanitize-recover=undefined,float-divide-by-zero,alignment,float-cast-overflow"
    )
    set(CMAKE_CXX_FLAGS_RELEASE
        "${CMAKE_CXX_FLAGS_RELEASE} -fno-omit-frame-pointer -fsanitize=undefined,float-divide-by-zero,alignment,float-cast-overflow -fno-sanitize-recover=undefined,float-divide-by-zero,alignment,float-cast-overflow"
    )
else()
    message(STATUS "omath: Undefined behavior sanitizer disabled.")
endif()
