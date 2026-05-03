add_subdirectory(example_barycentric)
add_subdirectory(example_glfw3)
add_subdirectory(example_proj_mat_builder)
add_subdirectory(example_signature_scan)
add_subdirectory(example_hud)

if(OMATH_ENABLE_HOOKING AND WIN32)
    # Requires imgui with dx9-binding, dx11-binding, dx12-binding, win32-binding.
    # Install via: vcpkg install imgui[dx9-binding,dx11-binding,dx12-binding,win32-binding]
    find_package(imgui CONFIG QUIET)
    if(imgui_FOUND)
        add_subdirectory(example_dx9_hook)
        add_subdirectory(example_dx11_hook)
        add_subdirectory(example_dx12_hook)
    else()
        message(STATUS "[omath] imgui not found — DX hook examples skipped")
    endif()
endif()

if(OMATH_ENABLE_VALGRIND)
    omath_setup_valgrind(example_projection_matrix_builder)
    omath_setup_valgrind(example_signature_scan)
    omath_setup_valgrind(example_glfw3)
endif()
