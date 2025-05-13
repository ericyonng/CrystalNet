#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "bson::static" for configuration "Debug"
set_property(TARGET bson::static APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(bson::static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/bson2.lib"
  )

list(APPEND _cmake_import_check_targets bson::static )
list(APPEND _cmake_import_check_files_for_bson::static "${_IMPORT_PREFIX}/lib/bson2.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
