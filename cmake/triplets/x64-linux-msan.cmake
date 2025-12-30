set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_CMAKE_SYSTEM_NAME Linux)

set(VCPKG_CXX_FLAGS "-stdlib=libc++ -fsanitize=memory -fno-omit-frame-pointer -g -O2 -DNDEBUG")
set(VCPKG_C_FLAGS "-fsanitize=memory -fno-omit-frame-pointer -g -O2 -DNDEBUG")
set(VCPKG_LINKER_FLAGS "-stdlib=libc++ -fsanitize=memory")
