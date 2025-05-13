include(CMakeFindDependencyMacro)
find_dependency(bson 2.0.0)
include("${CMAKE_CURRENT_LIST_DIR}/bsoncxx_targets.cmake")
