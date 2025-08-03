
if (NOT VCPKG_LIB_ARCH)
    set(VCPKG_LIB_ARCH "x64")
endif ()

if (NOT VCPKG_CONFIGURED)
    set(VCPKG_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/extlibs/vcpkg")

    if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/extlibs")
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/extlibs")
    endif ()

    if (NOT EXISTS "${VCPKG_ROOT}")
        message(STATUS "${VCPKG_ROOT} cannot find vcpkg directory. Executing git clone, please wait...")
        execute_process(
                COMMAND "git" "clone" "https://github.com/microsoft/vcpkg.git"
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/extlibs"
                OUTPUT_VARIABLE CLONE_OUTPUT
                ERROR_VARIABLE CLONE_ERROR
                RESULT_VARIABLE CLONE_STATUS
        )

        if (NOT CLONE_STATUS EQUAL 0)
            message(FATAL_ERROR "VCPKG has not been cloned. Git output:\n${CLONE_OUTPUT}\nGit error:\n${CLONE_ERROR}")
        endif ()
    endif ()

    if (WIN32)
        set(VCPKG_TARGET_TRIPLET "${VCPKG_LIB_ARCH}-windows-static")
        set(VCPKG_BINARY "${VCPKG_ROOT}/vcpkg.exe")
    elseif (UNIX)
        set(VCPKG_TARGET_TRIPLET "${VCPKG_LIB_ARCH}-linux")
        set(VCPKG_BINARY "${VCPKG_ROOT}/vcpkg")
    endif ()

    set(VCPKG_CONFIGURED TRUE)

    if (NOT EXISTS "${VCPKG_BINARY}")
        message(STATUS "${VCPKG_BINARY} not found, bootstrapping vcpkg, please wait...")
        if (WIN32 AND EXISTS "${VCPKG_ROOT}/bootstrap-vcpkg.bat")
            execute_process(
                    COMMAND "powershell" "-Command" "${VCPKG_ROOT}/bootstrap-vcpkg.bat"
                    WORKING_DIRECTORY "${VCPKG_ROOT}"
                    OUTPUT_VARIABLE BOOTSTRAP_OUTPUT
                    ERROR_VARIABLE BOOTSTRAP_ERROR
                    RESULT_VARIABLE BOOTSTRAP_STATUS
            )
        elseif (UNIX AND EXISTS "${VCPKG_ROOT}/bootstrap-vcpkg.sh")
            execute_process(
                    COMMAND "sh" "${VCPKG_ROOT}/bootstrap-vcpkg.sh"
                    WORKING_DIRECTORY "${VCPKG_ROOT}"
                    OUTPUT_VARIABLE BOOTSTRAP_OUTPUT
                    ERROR_VARIABLE BOOTSTRAP_ERROR
                    RESULT_VARIABLE BOOTSTRAP_STATUS
            )
        else ()
            message(FATAL_ERROR "Cannot find bootstrap script to get vcpkg executable")
        endif ()

        if (NOT BOOTSTRAP_STATUS EQUAL 0)
            message(FATAL_ERROR "VCPKG bootstrap failed. Output:\n${BOOTSTRAP_OUTPUT}\nError:\n${BOOTSTRAP_ERROR}")
        endif ()
    endif ()
    set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    message(STATUS "VCPKG configured with no errors")
endif ()

# --- helper: strip the feature part (everything after the first “[”) -------------
function(_vcpkg_get_base_package FULL_SPEC OUT_VAR)
    # e.g. imgui[dx11,dx12]  --> imgui
    string(REGEX REPLACE "\\[.*" "" _base "${FULL_SPEC}")
    set(${OUT_VAR} "${_base}" PARENT_SCOPE)
endfunction()

# --- install a single package spec ------------------------------------------------
function(vcpkg_install PackageSpec)
    string(REPLACE ";" "" PackageSpec "${PackageSpec}")
    message(STATUS "Installing ${PackageSpec}")
    execute_process(
            COMMAND "${VCPKG_BINARY}" "install"
            "${PackageSpec}:${VCPKG_TARGET_TRIPLET}"
            "--clean-after-build"
            RESULT_VARIABLE INSTALL_RES
            OUTPUT_VARIABLE INSTALL_OUTPUT
            ERROR_VARIABLE INSTALL_ERROR
            WORKING_DIRECTORY "${VCPKG_ROOT}"
    )
    if (NOT INSTALL_RES EQUAL 0)
        message(FATAL_ERROR "VCPKG install failed for ${PackageSpec}.\n"
                "Output:\n${INSTALL_OUTPUT}\nError:\n${INSTALL_ERROR}")
    endif ()
endfunction()

# --- install-if-needed for many specs --------------------------------------------
function(vcpkg_install_if_not_found)
    foreach (PkgSpec IN LISTS ARGN)
        # Figure out the directory that vcpkg puts headers / cmake files into
        _vcpkg_get_base_package("${PkgSpec}" BasePkg)            # → imgui
        set(_pkg_path "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/share/${BasePkg}")

        if (NOT EXISTS "${_pkg_path}")
            vcpkg_install("${PkgSpec}")          # install with features unchanged
        else ()
            message(STATUS "Library \"${PkgSpec}\" already present (found ${_pkg_path})")
        endif ()
    endforeach ()
endfunction()
