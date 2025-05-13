#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "mongoc::static" for configuration "Debug"
set_property(TARGET mongoc::static APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(mongoc::static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/mongoc2.lib"
  )

list(APPEND _cmake_import_check_targets mongoc::static )
list(APPEND _cmake_import_check_files_for_mongoc::static "${_IMPORT_PREFIX}/lib/mongoc2.lib" )

# Import target "mongoc::shared" for configuration "Debug"
set_property(TARGET mongoc::shared APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(mongoc::shared PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/lib/mongoc2.dll.lib"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/bin/mongoc2.dll"
  )

list(APPEND _cmake_import_check_targets mongoc::shared )
list(APPEND _cmake_import_check_files_for_mongoc::shared "${_IMPORT_PREFIX}/lib/mongoc2.dll.lib" "${_IMPORT_PREFIX}/bin/mongoc2.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
