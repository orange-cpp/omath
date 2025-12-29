# cmake/Valgrind.cmake

if(DEFINED __OMATH_VALGRIND_INCLUDED)
    return()
endif()
set(__OMATH_VALGRIND_INCLUDED TRUE)

find_program(VALGRIND_EXECUTABLE valgrind)
option(OMATH_ENABLE_VALGRIND "Enable Valgrind target for memory checking" ON)

if(OMATH_ENABLE_VALGRIND AND NOT TARGET valgrind_all)
    add_custom_target(valgrind_all)
endif()

function(omath_setup_valgrind TARGET_NAME)
    if(NOT OMATH_ENABLE_VALGRIND)
        return()
    endif()

    if(NOT VALGRIND_EXECUTABLE)
        message(WARNING "OMATH_ENABLE_VALGRIND is ON, but 'valgrind' executable was not found.")
        return()
    endif()

    set(VALGRIND_FLAGS 
        --leak-check=full 
        --show-leak-kinds=all
        --track-origins=yes 
        --error-exitcode=99
    )

    set(VALGRIND_TARGET "valgrind_${TARGET_NAME}")

    if(NOT TARGET ${VALGRIND_TARGET})
        add_custom_target(${VALGRIND_TARGET}
            DEPENDS ${TARGET_NAME}
            COMMAND ${VALGRIND_EXECUTABLE} ${VALGRIND_FLAGS} $<TARGET_FILE:${TARGET_NAME}>
            WORKING_DIRECTORY $<TARGET_FILE_DIR:${TARGET_NAME}>
            COMMENT "Running Valgrind memory check on ${TARGET_NAME}..."
            USES_TERMINAL
        )

        add_dependencies(valgrind_all ${VALGRIND_TARGET})
    endif()
endfunction()
