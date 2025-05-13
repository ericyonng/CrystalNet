#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "bson::shared" for configuration "Debug"
set_property(TARGET bson::shared APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(bson::shared PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/lib/bson2.dll.lib"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/bin/bson2.dll"
  )

list(APPEND _cmake_import_check_targets bson::shared )
list(APPEND _cmake_import_check_files_for_bson::shared "${_IMPORT_PREFIX}/lib/bson2.dll.lib" "${_IMPORT_PREFIX}/bin/bson2.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
