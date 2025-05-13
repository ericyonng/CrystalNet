#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "mongo::bsoncxx_static" for configuration "Debug"
set_property(TARGET mongo::bsoncxx_static APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(mongo::bsoncxx_static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/bsoncxx-static-dti-x64-v143-mdd.lib"
  )

list(APPEND _cmake_import_check_targets mongo::bsoncxx_static )
list(APPEND _cmake_import_check_files_for_mongo::bsoncxx_static "${_IMPORT_PREFIX}/lib/bsoncxx-static-dti-x64-v143-mdd.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
