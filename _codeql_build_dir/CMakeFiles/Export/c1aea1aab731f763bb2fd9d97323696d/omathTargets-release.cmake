#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "omath::omath" for configuration "Release"
set_property(TARGET omath::omath APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(omath::omath PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libomath.a"
  )

list(APPEND _cmake_import_check_targets omath::omath )
list(APPEND _cmake_import_check_files_for_omath::omath "${_IMPORT_PREFIX}/lib/libomath.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
