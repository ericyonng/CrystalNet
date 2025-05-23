#[[
    CMake package file for the mongoc library.

    This file globs and includes all "*-targets.cmake" files in the containing
    directory, and intends that those files define the actual mongoc targets.

    This file also defines a `mongoc::mongoc` target, which redirects to either
    `mongoc::static` or `mongoc::shared` depending on what type of library is
    available and can be controlled with an import-time CMake setting.

    If the installation has a static library, it is named `mongoc::static`. If
    the installation has a shared (dynamic) library, it is named `mongoc::shared`.
]]

# The version of the mongoc package (comes from the project() call in the orginal project)
set(__mongoc_package_version [[2.0.0]])
set(__mongoc_sasl_backend [[SSPI]])
set(__mongoc_tls_package [[NO]])
set(__mongoc_uses_bundled_utf8proc "ON")
# Announce this one as a public var:
set(MONGOC_TLS_BACKEND [[SecureChannel]])

# Check for missing components before proceeding. We don't provide any, so we
# should generate an error if the caller requests any *required* components.
set(missing_required_components)
foreach(comp IN LISTS mongoc_FIND_COMPONENTS)
    if(mongoc_FIND_REQUIRED_${comp})
        list(APPEND missing_required_components "${comp}")
    endif()
endforeach()

if(missing_required_components)
    list(JOIN missing_required_components ", " components)
    set(mongoc_FOUND FALSE)
    set(mongoc_NOT_FOUND_MESSAGE "The package version is compatible, but is missing required components: ${components}")
    # Stop now. Don't generate any imported targets
    return()
endif()

# d8888b. d88888b d8888b. d88888b d8b   db d8888b. d88888b d8b   db  .o88b. d888888b d88888b .d8888.
# 88  `8D 88'     88  `8D 88'     888o  88 88  `8D 88'     888o  88 d8P  Y8   `88'   88'     88'  YP
# 88   88 88ooooo 88oodD' 88ooooo 88V8o 88 88   88 88ooooo 88V8o 88 8P         88    88ooooo `8bo.
# 88   88 88~~~~~ 88~~~   88~~~~~ 88 V8o88 88   88 88~~~~~ 88 V8o88 8b         88    88~~~~~   `Y8b.
# 88  .8D 88.     88      88.     88  V888 88  .8D 88.     88  V888 Y8b  d8   .88.   88.     db   8D
# Y8888D' Y88888P 88      Y88888P VP   V8P Y8888D' Y88888P VP   V8P  `Y88P' Y888888P Y88888P `8888Y'
# Import dependencies
include(CMakeFindDependencyMacro)
get_filename_component(__parent_dir "${CMAKE_CURRENT_LIST_DIR}" DIRECTORY)
# Also import the `bson` package, to ensure its targets are also available
find_dependency(bson "${__mongoc_package_version}" HINTS ${__parent_dir})

# QUIET arg for finding dependencies:
unset(__quiet)
if(${CMAKE_FIND_PACKAGE_NAME}_FIND_QUIETLY)
  set(__quiet QUIET)
endif()

# Find libutf8proc (if we linked to an external library)
if(NOT __mongoc_uses_bundled_utf8proc AND NOT TARGET PkgConfig::PC_UTF8PROC)
    # libmongoc was compiled against an external utf8proc and links against a
    # FindPkgConfig-generated IMPORTED target. Find that package and generate that
    # imported target here:
    find_dependency(PkgConfig)
    pkg_check_modules(PC_UTF8PROC ${__quiet} libutf8proc IMPORTED_TARGET GLOBAL)
    if(NOT PC_UTF8PROC_FOUND)
        # Handle if it wasn't found (find_dependency would usually do this for us,
        # but pkg_check_modules() does not)
        set(mongoc_FOUND FALSE)
        set(mongoc_NOT_FOUND_MESSAGE "We were unable to find the required libutf8proc package with pkg-config")
        return()
    endif()
endif()

# If we need to import a TLS package for our imported targets, do that now:
if(__mongoc_tls_package)
    find_dependency("${__mongoc_tls_package}")
endif()

# Find dependencies for SASL
if(__mongoc_sasl_backend STREQUAL "Cyrus")
    # We need libsasl2. The find-module should be installed within this package.
    # temporarily place it on the module search path:
    set(__prev_path "${CMAKE_MODULE_PATH}")
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/3rdParty")
    find_dependency(SASL2 2.0)
    set(CMAKE_MODULE_PATH "${__prev_path}")
endif()

# Import the target files that will be installed alongside this file. Only the
# targets of libraries that were actually installed alongside this file will be imported
file(GLOB __targets_files "${CMAKE_CURRENT_LIST_DIR}/*-targets.cmake")
foreach(__file IN LISTS __targets_files)
    include("${__file}")
endforeach()

# The library type that is linked with `mongoc::mongoc`
set(__default_lib_type SHARED)
if(TARGET mongoc::static)
    # If static is available, set it as the default library type
    set(__default_lib_type STATIC)
endif()

# Allow the user to tweak what library type is linked for `mongoc::mongoc`
set(MONGOC_DEFAULT_IMPORTED_LIBRARY_TYPE "${__default_lib_type}"
    CACHE STRING "The default library type that is used when linking against 'mongoc::mongoc' (either SHARED or STATIC, requires that the package was built with the appropriate library type)")
set_property(CACHE MONGOC_DEFAULT_IMPORTED_LIBRARY_TYPE PROPERTY STRINGS SHARED STATIC)

if(NOT TARGET mongoc::mongoc)  # Don't redefine the target if we were already included
    string(TOLOWER "${MONGOC_DEFAULT_IMPORTED_LIBRARY_TYPE}" __type)
    add_library(mongoc::mongoc IMPORTED INTERFACE)
    set_property(TARGET mongoc::mongoc APPEND PROPERTY INTERFACE_LINK_LIBRARIES mongoc::${__type})
endif()
