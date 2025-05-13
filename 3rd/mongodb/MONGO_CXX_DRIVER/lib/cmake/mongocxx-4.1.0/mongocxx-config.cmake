include(CMakeFindDependencyMacro)
find_dependency(mongoc 2.0.0)
find_dependency(bsoncxx 4.1.0)
include("${CMAKE_CURRENT_LIST_DIR}/mongocxx_targets.cmake")
